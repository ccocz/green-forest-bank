//
// Created by resul on 07.12.2021.
//

//todo: check for memory corruption

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

void get_current_date(char* date) {
    const time_t t = time(NULL);
    struct tm* lt = localtime(&t);
    sprintf(date, "%d.%d.%d", lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900);
}

void select_user(char* user_id) {
    printf("Enter user ID: ");
    scanf("%s", user_id);
    // check if exists
}

void add_credit(char* user_id) {
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
    strcpy(user_dir, "/home/bank/credits/");
    strcat(user_dir, id);
    strcat(user_dir, "/");

    DIR* d;
    d = opendir(user_dir);
    if (!d) {
        fprintf(stderr, "Couldn't open directory");
        exit(1);
    }
    struct dirent* dir;
    int no_credits = 0;
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_REG) {
            printf("%s\n", dir->d_name);
            no_credits++;
        }
    }
    closedir(d);

    char new_credit[MAX_FILE_LENGTH];
    strcpy(new_credit, id);
    strcat(new_credit, "-credit-");
    sprintf(new_credit + strlen(new_credit), "%d", no_credits + 1);

    char credit_path[MAX_PATH_LENGTH];
    strcpy(credit_path, user_dir);
    strcat(credit_path, new_credit);

    printf("new credit file name: %s\n", credit_path);

    FILE* new_credit_file = fopen(credit_path, "w");
    if (new_credit_file == NULL) {
        fprintf(stderr, "Couldn't create new credit file %s for user %s", new_credit, id);
        exit(1);
    }

    // get name and surname

    struct passwd* pw = getpwnam(id);

    if (pw == NULL) {
        fprintf(stderr, "getpwnam failed %s", id);
        exit(1);
    }

    printf("user info: %s", pw->pw_gecos);

    fprintf(new_credit_file, "Name: %s\n", pw->pw_gecos);
    fprintf(new_credit_file, "Number: %d\n", no_credits + 1);
    fprintf(new_credit_file, "Sum: %d\n", sum);
    fprintf(new_credit_file, "Date: %s\n", date);
    fprintf(new_credit_file, "Procent: %d\n", percentage);

    fclose(new_credit_file);
}

void add_deposit(char* user_id) {
}

void add(char* user_id) {
    printf("Choose credit (1) or deposit (2): ");
    int option;
    scanf("%d", &option);
    if (option == 1) {
        add_credit(user_id);
    } else if (option == 2) {
        add_deposit(user_id);
    } else {
        add(user_id);
    }
}

void display() {
}

void modify() {
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
            case 2:
                break;
            case 3:
                add(user_id_set ? user_id : NULL);
            case 4:
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
