#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/wait.h>
#include <errno.h>

int main() {
    FILE *fpin;
    char msg[BUFSIZ];
    if ((fpin = popen("echo abCDefGHIjKlMnopqrstuvWXYZ", "r")) == NULL) {
        perror("Failed to open process for reading with popen");
        exit(-1);
    }
    while (fgets(msg, BUFSIZ, fpin) != NULL) {
        for (int i = 0; msg[i] != 0; i++){
            msg[i] = toupper(msg[i]);
        }
        printf("%s", msg);
    }
    if (!feof(fpin)) {
        fprintf(stderr, "End of pipe not reached.\n");
        exit(-1);
    } else if (ferror(fpin)) {
        fprintf(stderr, "An error occurred while reading pipe.\n");
        exit(-1);
    }
    int status = pclose(fpin);
    if (status == -1) {
        perror("Failed to close process stream with pclose");
        exit(-1);
    } else if (WIFEXITED(status)) {
        printf("Child from pipe finished with exit status %d\n", WEXITSTATUS(status));
    }
    return 0;
}
