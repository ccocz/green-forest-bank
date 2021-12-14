//
// Created by resul on 07.12.2021.
//

#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>

#define TIME_DIFF_SEC 15
#define USER_ID_LEN 20
#define USER_NAME_LEN 50
#define ASSET_LINE_LEN 100

#define MAX_PATH_LENGTH 100
#define MAX_FILE_LENGTH 30

#define SUM_PERIOD_LEN 4
#define PERC_PERIOD_LEN 3

enum period_type {SUM, PERCENTAGE};

struct period {
    char* sum;
    char* percentage;
    char* start_date;
    char* end_date;
    enum period_type type;
};

struct asset {
    char name[USER_NAME_LEN];
    int number;
    size_t periods_length;
    struct period* periods;
};

static struct pam_conv login_conv = {
        misc_conv,
        NULL
};

bool has_prefix(char* str, char* prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

bool is_valid_date(char* date, int* year, int* month, int* day) {
    int pos;
    return sscanf(date, "%d.%d.%d %n", day, month, year, &pos) == 3
           && pos == strlen(date)
           && (*month >= 1 && *month <= 12)
           && (*day >= 1 && *day <= 31);
}

void handle_login() {
    pam_handle_t* pamh = NULL;
    int retval;
    char* username = NULL;

    retval = pam_start("login", username, &login_conv, &pamh);
    if (pamh == NULL || retval != PAM_SUCCESS) {
        fprintf(stderr, "Error when starting PAM: %d\n", retval);
        exit(1);
    }

    retval = pam_authenticate(pamh, 0);
    if (retval != PAM_SUCCESS) {
        fprintf(stderr, "Login failed\n");
        exit(2);
    }

    time_t in;
    time_t current = time(NULL);
    fprintf(stderr, "system time (in seconds after epoch): ");
    scanf("%ld", &in);
    if (current == (time_t)(-1)) {
        fprintf(stderr, "Couldn't get system time\n");
        exit(1);
    }

    if (labs(current - in) >= TIME_DIFF_SEC) {
        fprintf(stderr, "Login failed\n");
        exit(2);
    }
    puts("Logged in");
    pam_end(pamh, PAM_SUCCESS);
}

bool select_user(char* user_id) {
    printf("Enter user ID: ");
    fflush(stdout);
    // initially width had no limits, then I put 19 = USER_ID_LEN - 1 (cppcheck)
    scanf("%19s", user_id);

    if (getpwnam(user_id) == NULL) {
        fprintf(stderr, "User cannot be found in the system\n");
        return false;
    }

    printf("Selected user %s\n", user_id);
    return true;
}

void add_asset(char* user_id, char* asset) {
    char id[USER_ID_LEN];
    if (user_id == NULL) {
        select_user(id);
    } else {
        printf("Currently selected user %s, change? (\"yes\" or \"no\") ", user_id);
        fflush(stdout);
        char ans[5];
        scanf("%4s", ans); // cppcheck, here and all below
        if (!strcmp(ans, "yes")) {
            select_user(id);
        } else {
            strncpy(id, user_id, strlen(user_id));
            id[strlen(user_id)] = '\0';
        }
    }

    printf("Selected user %s\n", id);
    double sum, percentage;
    char date[ASSET_LINE_LEN];
    printf("Enter the sum: "), fflush(stdout), scanf("%lf", &sum);
    printf("Enter start date of the first billing period (in DD.MM.YYYY format): "), fflush(stdout), scanf("%99s", date);
    while (1) {
        int year, month, day;
        if (!is_valid_date(date, &year, &month, &day)) {
            printf("Enter start date of the first billing period (in DD.MM.YYYY format): "), fflush(stdout), scanf("%99s", date);
        } else {
            break;
        }
    }
    printf("Enter percentage: "), fflush(stdout), scanf("%lf", &percentage);

    char user_dir[MAX_PATH_LENGTH];
    strcpy(user_dir, "/home/bank/");
    strcat(user_dir, asset);
    strcat(user_dir, "s/");
    strcat(user_dir, id);
    strcat(user_dir, "/");

    DIR* d;
    d = opendir(user_dir);
    if (!d) {
        fprintf(stderr, "Couldn't open directory %s\n", user_dir);
        exit(1);
    }
    struct dirent* dir;
    int no_assets = 0;
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_REG) {
            printf("%s\n", dir->d_name);
            no_assets++;
        }
    }
    closedir(d);

    char new_asset[MAX_FILE_LENGTH];
    strcpy(new_asset, id);
    strcat(new_asset, "-");
    strcat(new_asset, asset);
    strcat(new_asset, "-");
    sprintf(new_asset + strlen(new_asset), "%d", no_assets + 1);

    char asset_path[MAX_PATH_LENGTH];
    strcpy(asset_path, user_dir);
    strcat(asset_path, new_asset);

    printf("new asset file name: %s\n", asset_path);

    FILE* new_asset_file = fopen(asset_path, "w");
    if (new_asset_file == NULL) {
        fprintf(stderr, "Couldn't create new asset file %s for user %s\n", new_asset, id);
        exit(1);
    }

    struct passwd* pw = getpwnam(id);

    if (pw == NULL) {
        fprintf(stderr, "getpwnam failed %s\n", id);
        exit(1);
    }

    fprintf(new_asset_file, "Name: %s\n", pw->pw_gecos);
    fprintf(new_asset_file, "Number: %d\n", no_assets + 1);
    fprintf(new_asset_file, "Sum: %g\n", sum);
    fprintf(new_asset_file, "Date: %s\n", date);
    fprintf(new_asset_file, "Procent: %g\n", percentage);

    if (chown(asset_path, pw->pw_uid, pw->pw_gid) == -1) {
        fprintf(stderr, "chown failed %s %u %u\n", asset_path, pw->pw_uid, pw->pw_gid);
        exit(1);
    }

    fclose(new_asset_file);
}

void add(char* user_id) {
    int option = 0;

    do {
        printf("Choose credit (1) or deposit (2): ");
        fflush(stdout);
    } while ((scanf("%d", &option) != 1) || !(option == 1 || option == 2));

    if (option == 1) {
        add_asset(user_id, "credit");
    } else {
        add_asset(user_id, "deposit");
    }

    printf("\nAdded\n");
}

char** get_all_lines(char* path, char* file, size_t* pos_ret) {
    char file_path[MAX_PATH_LENGTH];
    strcpy(file_path, path);

    if (file != NULL) {
        strcat(file_path, "/");
        strcat(file_path, file);
    }

    FILE* content = fopen(file_path, "r");
    if (content == NULL) {
        fprintf(stderr, "Couldn't open file %s\n", file_path);
        exit(1);
    }

    char** lines = NULL;
    size_t no_lines = 0;
    size_t pos = 0;

    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, content)) != -1) {
        if (pos == no_lines) {
            no_lines = (no_lines + 1) * 2;
            lines = realloc(lines, sizeof(char*) * no_lines);
            // null check added later, error found by analysis program cppcheck
            if (lines == NULL) {
                fprintf(stderr, "Cannot allocate memory for lines\n");
                exit(2);
            }
        }
        lines[pos++] = strdup(line);
    }

    *pos_ret = pos;
    free(line);
    fclose(content);
    return lines;
}

void display_asset(char* path, char* file) {
    size_t pos = 0;
    char** lines = get_all_lines(path, file, &pos);

    size_t start = 2;

    struct period** periods = NULL;
    size_t len_periods = 0;
    ssize_t pos_periods = 0;

    while (start < pos) {
        struct period* next_period = malloc(sizeof(struct period));
        if (has_prefix(lines[start], "Sum:")) {
            next_period->type = SUM;
            next_period->sum = lines[start];
            next_period->start_date = lines[start + 1];
            next_period->percentage = lines[start + 2];
            if (start + SUM_PERIOD_LEN < pos && has_prefix(lines[start + SUM_PERIOD_LEN], "Sum:")) {
                next_period->end_date = lines[start + 3];
                start += SUM_PERIOD_LEN;
            } else {
                next_period->end_date = NULL;
                start += SUM_PERIOD_LEN - 1;
            }
        } else {
            next_period->type = PERCENTAGE;
            next_period->start_date = lines[start];
            if (start + 1 == pos) {
                next_period->percentage = NULL;
            } else {
                next_period->percentage = lines[start + 1];
            }
            if (start + PERC_PERIOD_LEN < pos && has_prefix(lines[start + PERC_PERIOD_LEN], "Sum:")) {
                next_period->end_date = lines[start + 2];
                start += PERC_PERIOD_LEN;
            } else {
                next_period->end_date = NULL;
                start += PERC_PERIOD_LEN - 1;
            }
        }
        if (pos_periods == len_periods) {
            len_periods = (len_periods + 1) * 2;
            periods = realloc(periods, sizeof(struct period) * len_periods);
            // null check added later, error found by analysis program cppcheck
            if (periods == NULL) {
                fprintf(stderr, "Cannot allocate memory for periods\n");
                exit(2);
            }
        }
        periods[pos_periods++] = next_period;
    }

    printf("%s%s", lines[0], lines[1]);

    for (ssize_t i = pos_periods - 1; i >= 0; i--) {
        if (periods == NULL) {
            break;
        }
        if (periods[i]->type == SUM) {
            printf("%s%s%s", periods[i]->sum, periods[i]->start_date, periods[i]->percentage);
            if (periods[i]->end_date != NULL) {
                printf("%s", periods[i]->end_date);
            }
        } else {
            printf("%s", periods[i]->start_date);
            if (periods[i]->percentage != NULL) {
                printf("%s", periods[i]->percentage);
            }
            if (periods[i]->end_date != NULL) {
                printf("%s", periods[i]->end_date);
            }
        }
        puts("");
    }

    for (size_t i = 0; i < pos; i++) {
        free(lines[i]);
    }
    free(lines);

    for (size_t i = 0; i < pos_periods; i++) {
        free(periods[i]);
    }
    free(periods);

}

void display_assets(char* path) {
    DIR* d;
    d = opendir(path);
    if (!d) {
        fprintf(stderr, "Couldn't open directory %s\n", path);
        exit(1);
    }
    struct dirent* dir;
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_REG) {
            printf("%s-contents\n", dir->d_name);
            display_asset(path, dir->d_name);
            puts("");
        }
    }
    closedir(d);
}

void display(char* user_id) {

    char base_dir[MAX_PATH_LENGTH];

    sprintf(base_dir, "/home/bank/credits/%s", user_id);
    display_assets(base_dir);

    sprintf(base_dir, "/home/bank/deposits/%s", user_id);
    display_assets(base_dir);
}

bool earlier(char* date1, char* date2) {
    int year1, month1, day1;
    int year2, month2, day2;

    if (!is_valid_date(date1, &year1, &month1, &day1)
        || !is_valid_date(date2, &year2, &month2, &day2)) {
        return false;
    }

    return year1 < year2
        || (year1 == year2 && month1 < month2)
        || (year1 == year2 && month1 == month2 && day1 < day2);
}

bool cmp_last_date(char* path, char* file, char* date) {

    size_t pos;
    char** lines = get_all_lines(path, file, &pos);

    bool ans = true;

    for (size_t i = 0; i < pos; i++) {
        if (has_prefix(lines[i], "Date: ") && !earlier(lines[i] + strlen("Date: "), date)) {
            ans = false;
        }
        free(lines[i]);
    }
    free(lines);
    return ans;
}

void modify(char* user_id) {
    char option[10];
    input_type:
    printf("Type \"credit\" or \"deposit\": ");
    fflush(stdout);
    scanf("%9s", option);

    if (!(strcasecmp(option, "credit") == 0 || strcasecmp(option, "deposit") == 0)) {
        goto input_type;
    }

    int number;
    printf("Number of chosen asset: ");
    fflush(stdout);
    scanf("%d", &number);

    char asset_file[MAX_FILE_LENGTH];
    sprintf(asset_file, "%s-%s-%d", user_id, option, number);

    char user_dir[MAX_PATH_LENGTH];
    strcpy(user_dir, "/home/bank/");
    strcat(user_dir, option);
    strcat(user_dir, "s/");
    strcat(user_dir, user_id);
    strcat(user_dir, "/");
    strcat(user_dir, asset_file);

    FILE* edited = fopen(user_dir, "r");
    if (edited == NULL) {
        fprintf(stderr, "Credit/deposit doesn't exist\n");
        return;
    }
    fclose(edited);

    edited = fopen(user_dir, "a");
    if (edited == NULL) {
        fprintf(stderr, "Couldn't open file %s\n", user_dir);
        exit(1);
    }

    const int no_options = 3;
    const char* options[] = {"1. New sum position",
                             "2. New billing period",
                             "3. End billing period"};
    selection:
    for (size_t i = 0; i < no_options; i++) {
        puts(options[i]);
    }
    printf("Select operation (1-3): ");
    fflush(stdout);
    int selection;
    scanf("%d", &selection);
    double sum, percentage;
    char date[ASSET_LINE_LEN];
    switch (selection) {
        case 1:
            printf("Enter sum: "), fflush(stdout), scanf("%lf", &sum);
            printf("Starting date (in DD.MM.YYYY format): "), fflush(stdout), scanf("%99s", date);
            printf("Percentage: "), fflush(stdout), scanf("%lf", &percentage);
            if (!cmp_last_date(user_dir, NULL, date)) {
                fprintf(stderr, "Date is not later than starting or is ill-formatted\n");
                break;
            }
            fprintf(edited, "Date: %s\n", date);
            fprintf(edited, "Sum: %g\n", sum);
            fprintf(edited, "Date: %s\n", date);
            fprintf(edited, "Procent: %g\n", percentage);
            break;
        case 2:
            printf("Ending date (starting at the same time in DD.MM.YYYY format): "), fflush(stdout), scanf("%99s", date);
            printf("Percentage: "), fflush(stdout), scanf("%lf", &percentage);
            if (!cmp_last_date(user_dir, NULL, date)) {
                fprintf(stderr, "Date is not later than starting or is ill-formatted\n");
                break;
            }
            fprintf(edited, "Date: %s\n", date);
            fprintf(edited, "Procent: %g\n", percentage);
            break;
        case 3:
            printf("Ending date: "), fflush(stdout), scanf("%99s", date);
            if (!cmp_last_date(user_dir, NULL, date)) {
                fprintf(stderr, "Date is not later than starting or is ill-formatted\n");
                break;
            }
            fprintf(edited, "Date: %s\n", date);
            break;
        default:
            goto selection;
    }

    printf("\nModified\n");
    fclose(edited);

}

void main_menu() {
    const int no_options = 5;
    const char* options[] = {"\n1. Select customer",
                           "2. Display deposit/credit",
                           "3. Add deposit/credit",
                           "4. Modify deposit/credit",
                           "5. Quit\n"};
    char user_id[USER_ID_LEN];
    bool user_id_set = false;
    while (1) {
        for (size_t i = 0; i < no_options; i++) {
            puts(options[i]);
        }
        printf("Select operation (1-4): ");
        fflush(stdout);
        int selection;
        scanf("%d", &selection);
        switch (selection) {
            case 1:
                if (select_user(user_id)) {
                    user_id_set = true;
                }
                break;
            case 2:
                if (!user_id_set) {
                    printf("You need to first select user\n");
                    break;
                }
                display(user_id);
                break;
            case 3:
                add(user_id_set ? user_id : NULL);
                break;
            case 4:
                if (!user_id_set) {
                    printf("You need to first select user\n");
                    break;
                }
                modify(user_id);
                break;
            case 5:
                exit(0);
        }
    }

}

int main () {
    handle_login();
    main_menu();
    return 0;
}
