#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Not enough arguments\n");
        exit(1);
    }

    pid_t pid;
    int status;

    if ((pid = fork()) == -1) {
        perror("Fork failed");
        exit(1);

    } else if (pid == 0) {
        if (execvp(argv[1], &argv[1]) == -1) {
            perror("Execvp error");
            exit(1);
        }
    } else {
        if (wait(&status) == -1) {
            perror("Wait failed");
            exit(1);
        } else if (WIFEXITED(status)) {
            printf("%d\n", WEXITSTATUS(status));
        }
    }

    exit(0);
}
