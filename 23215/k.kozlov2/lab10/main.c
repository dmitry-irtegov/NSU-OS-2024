#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        if (execvp(argv[1], argv + 1) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else {
        int status;

        if (wait(&status) == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status)) {
            printf("Process finished with exit status: %d\n", WEXITSTATUS(status));
        }
    }

    exit(EXIT_SUCCESS);
}
