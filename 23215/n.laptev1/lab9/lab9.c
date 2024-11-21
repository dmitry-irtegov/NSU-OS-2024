#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char** argv) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Error in \"fork\" ");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        char* args[] = {"/bin/cat", "lab9.c", NULL};
        if (execv("/bin/cat", args) == -1) {
            perror("Error in \"execv\" ");
            exit(EXIT_FAILURE);
        }
    }

    if (pid > 0) {
        if (wait(NULL) != pid) {
            perror("Error in \"wait\"");
            exit(EXIT_FAILURE);
        }
        printf("%s","Last String\n.");
    }
    return 0;
}
