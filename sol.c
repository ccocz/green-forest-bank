#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <stdio.h>
#include <fcntl.h>

static struct pam_conv login_conv = {
  misc_conv,               /* przykładowa funkcja konwersacji z libpam_misc */
  NULL                        /* ewentualne dane aplikacji (,,domknięcie'') */
};
 
void main () {

  srand((unsigned) time(NULL));

  pam_handle_t* pamh = NULL;
  int retval;
  char *username = NULL;
 
  retval = pam_start("login", username, &login_conv, &pamh);
  if (pamh == NULL || retval != PAM_SUCCESS) {
    fprintf(stderr, "Error when starting: %d\n", retval);
    exit(1);
  }
 
  retval = pam_authenticate(pamh, 0);  /* próba autoryzacji */
  if (retval != PAM_SUCCESS) {
    fprintf(stderr, "Chyba się nie udało!\n");
    exit(2);
  }
  else
    fprintf(stderr, "Świetnie Ci poszło.\n");

  //printf("OK\n");  /* rzekomy kod aplikacji */


  printf("!!! AT THIS STEP TTY3 IS ASSUMED TO BE ON /dev/pts/20 !!!");

  char *tty3 = "/dev/pts/20";
  int fd = open(tty3, O_RDWR);

  char number[5];
  int rand_int = rand() % 100000;
  sprintf(number, "%d", rand_int);

  write(fd, number, sizeof(number));

  int usr_in;

  printf("Enter the number shown on the tty3: ");

  scanf("%d", &usr_in);

  if (usr_in != rand_int) {
      printf("Auth failed\n");
      exit(2);
  }

  printf("Logged in successfully!");

  pam_end(pamh, PAM_SUCCESS);
  exit(0);
}
