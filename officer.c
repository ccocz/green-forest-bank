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
#define DATE_LEN 20

#define MAX_PATH_LENGTH 100
#define MAX_FILE_LENGTH 30

static struct pam_conv login_conv = {
        misc_conv,
        NULL
};

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
    fprintf(stderr, "system time: ");
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
        }
    }

    printf("finally selected user %s\n", id);
    int sum, percentage;
    char date[DATE_LEN];
    printf("Enter the sum: "), scanf("%d", &sum);
    printf("Enter start date of the first billing period: "), scanf("%s", date);
    printf("Enter percentage: "), scanf("%d", &percentage);

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
    fprintf(new_asset_file, "Sum: %d\n", sum);
    fprintf(new_asset_file, "Date: %s\n", date);
    fprintf(new_asset_file, "Procent: %d\n", percentage);

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

void display() {
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
    int sum, percentage;
    char date[DATE_LEN];
    switch (selection) {
        case 1:
            printf("Enter sum: "), scanf("%d", &sum);
            printf("Starting date: "), scanf("%s", date);
            printf("Percentage: "), scanf("%d", &percentage);
            // todo: check if ending date is later than starting date
            fprintf(edited, "Date: %s\n", date);
            fprintf(edited, "Sum: %d\n", sum);
            fprintf(edited, "Date: %s\n", date);
            fprintf(edited, "Procent: %d\n", percentage);
            break;
        case 2:
            printf("Ending date (starting at the same time): "), scanf("%s", date);
            printf("Percentage: "), scanf("%d", &percentage);
            fprintf(edited, "Date: %s\n", date);
            fprintf(edited, "Procent: %d\n", percentage);
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
    //handle_login();
    main_menu();

    return 0;
}
