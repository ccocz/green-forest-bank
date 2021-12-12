//
// Created by resul on 07.12.2021.
//

/*
 * todo:
 * check for memory corruption
 * change credit/deposit file owners
 * sum to float
 * check for types long
 * add successful operation message
 * password visible
 * double less precision
 * user permissions for his/her own files
 * things left to do:
    implement officer and client connection
    prevent adding earlier date
    fix init issues
    firewall
    a mieliście może taki problem że sie w dockerze nie da naraz odpalić ssh i aplikacji sieciowej, bo może być tylko jeden CMD?
 */

/*
 * questions:
 * who is running this script?
 */

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

void handle_login() {
    pam_handle_t* pamh = NULL;
    int retval;
    char* username = NULL;

    retval = pam_start("login", username, &login_conv, &pamh);
    if (pamh == NULL || retval != PAM_SUCCESS) {
        fprintf(stderr, "Error when starting PAM: %d\n", retval);
        exit(1);
    }

    retval = pam_authenticate(pamh, 0);  /* próba autoryzacji */
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

void select_user(char* user_id) {
    printf("Enter user ID: ");
    scanf("%s", user_id);
    // check if exists
}

void add_asset(char* user_id, char* asset) {
    char id[USER_ID_LEN];
    if (user_id == NULL) {
        select_user(id);
    } else {
        printf("Currently selected user %s, change? ", user_id);
        char ans[5];
        scanf("%s", ans);
        if (!strcmp(ans, "yes")) {
            select_user(id);
        } else {
            strncpy(id, user_id, strlen(user_id));
            id[strlen(user_id)] = '\0';
        }
    }

    printf("finally selected user %s\n", id);
    double sum, percentage;
    char date[ASSET_LINE_LEN];
    printf("Enter the sum: "), scanf("%lf", &sum);
    printf("Enter start date of the first billing period: "), scanf("%s", date);
    printf("Enter percentage: "), scanf("%lf", &percentage);

    char user_dir[MAX_PATH_LENGTH];
    strcpy(user_dir, "/home/bank/");
    strcat(user_dir, asset);
    strcat(user_dir, "s/");
    strcat(user_dir, id);
    strcat(user_dir, "/");

    DIR* d;
    d = opendir(user_dir);
    if (!d) {
        fprintf(stderr, "Couldn't open directory %s", user_dir);
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
        fprintf(stderr, "Couldn't create new asset file %s for user %s", new_asset, id);
        exit(1);
    }

    struct passwd* pw = getpwnam(id);

    if (pw == NULL) {
        fprintf(stderr, "getpwnam failed %s", id);
        exit(1);
    }

    printf("user info: %s", pw->pw_gecos);

    fprintf(new_asset_file, "Name: %s\n", pw->pw_gecos);
    fprintf(new_asset_file, "Number: %d\n", no_assets + 1);
    fprintf(new_asset_file, "Sum: %g\n", sum);
    fprintf(new_asset_file, "Date: %s\n", date);
    fprintf(new_asset_file, "Procent: %g\n", percentage);

    fclose(new_asset_file);
}

void add(char* user_id) {
    printf("Choose credit (1) or deposit (2): ");
    int option;
    scanf("%d", &option);
    if (option == 1) {
        add_asset(user_id, "credit");
    } else if (option == 2) {
        add_asset(user_id, "deposit");
    } else {
        add(user_id);
    }
}

void display_asset(char* path, char* file) {
    char file_path[MAX_PATH_LENGTH];
    strcpy(file_path, path);
    strcat(file_path, "/");
    strcat(file_path, file);

    FILE* content = fopen(file_path, "r");
    if (content == NULL) {
        fprintf(stderr, "Couldn't open file %s", file_path);
        exit(1);
    }

    char** lines = NULL;
    size_t no_lines = 0;
    size_t pos = 0;

    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, content)) != -1) {
        //printf("len: %zu, line: %s\n", len, line);
        if (pos == no_lines) {
            no_lines = (no_lines + 1) * 2;
            lines = realloc(lines, sizeof(char*) * no_lines);
        }
        lines[pos++] = strdup(line);
        //printf("pos: %zu, line: %s\n", pos - 1, lines[pos - 1]);
        // why not free here?
    }

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
            next_period->percentage = lines[start + 1];
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
        }
        periods[pos_periods++] = next_period;
    }

    printf("%s%s", lines[0], lines[1]);

    for (ssize_t i = pos_periods - 1; i >= 0; i--) {
        if (periods[i]->type == SUM) {
            printf("%s%s%s", periods[i]->sum, periods[i]->start_date, periods[i]->percentage);
            if (periods[i]->end_date != NULL) {
                printf("%s", periods[i]->end_date);
            }
        } else {
            printf("%s%s", periods[i]->start_date, periods[i]->percentage);
            if (periods[i]->end_date != NULL) {
                printf("%s", periods[i]->end_date);
            }
        }
        puts("");
    }

    for (size_t i = 0; i < pos; i++) {
        //printf("%zu-th line: %s", i, lines[i]);
        free(lines[i]);
    }
    free(lines);
    free(line);

    for (size_t i = 0; i < pos_periods; i++) {
        free(periods[i]);
    }
    free(periods);

    fclose(content);
}

void display_assets(char* path) {
    DIR* d;
    d = opendir(path);
    if (!d) {
        fprintf(stderr, "Couldn't open directory %s", path);
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

void modify(char* user_id) {
    char option[10];
    printf("Type \"credit\" or \"deposit\": ");
    scanf("%s", option);

    int number;
    printf("Number of chosen asset: ");
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

    FILE* edited = fopen(user_dir, "a");
    if (edited == NULL) {
        fprintf(stderr, "Couldn't open file %s", user_dir);
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
    int selection;
    scanf("%d", &selection);
    double sum, percentage;
    char date[ASSET_LINE_LEN];
    switch (selection) {
        case 1:
            printf("Enter sum: "), scanf("%lf", &sum);
            printf("Starting date: "), scanf("%s", date);
            printf("Percentage: "), scanf("%lf", &percentage);
            // todo: check if ending date is later than starting date
            fprintf(edited, "Date: %s\n", date);
            fprintf(edited, "Sum: %g\n", sum);
            fprintf(edited, "Date: %s\n", date);
            fprintf(edited, "Procent: %g\n", percentage);
            break;
        case 2:
            printf("Ending date (starting at the same time): "), scanf("%s", date);
            printf("Percentage: "), scanf("%lf", &percentage);
            fprintf(edited, "Date: %s\n", date);
            fprintf(edited, "Procent: %g\n", percentage);
            break;
        case 3:
            printf("Ending date: "), scanf("%s", date);
            fprintf(edited, "Date: %s\n", date);
            break;
        default:
            goto selection;
    }

    fclose(edited);

}

void main_menu() {
    const int no_options = 5;
    const char* options[] = {"1. Select customer",
                           "2. Display deposit/credit",
                           "3. Add deposit/credit",
                           "4. Modify deposit/credit",
                           "5. Quit"};
    char user_id[USER_ID_LEN];
    bool user_id_set = false;
    while (1) {
        for (size_t i = 0; i < no_options; i++) {
            puts(options[i]);
        }
        printf("Select operation (1-4): ");
        int selection;
        scanf("%d", &selection);
        switch (selection) {
            case 1:
                select_user(user_id);
                user_id_set = true;
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
