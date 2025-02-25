#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

    if (argc == 1) {
        printf("No program to exec\n");
        exit(1);
    }

    char** arguments = NULL;
    arguments = malloc(sizeof(char*) * (argc));
    if (arguments == NULL) {
        perror("malloc error");
        exit(1);
    }
    arguments[0] = argv[1];
    for (int i = 1; i < argc - 1; i++) {
        arguments[i] = argv[i + 1];
    }
    arguments[argc - 1] = NULL;

    pid_t child_id, finished_procces;
    int status;

    child_id = fork();
    switch (child_id) {
    case 0:

        execvp(argv[1], arguments);
        perror("exec error");
        exit(1);

    case -1:
        perror("fork error");
        exit(1);

    default:
        finished_procces = wait(&status);

        if (finished_procces == -1) {
            perror("wait error");
            exit(1);
        }

        if (WIFEXITED(status)) {
            printf("Child`s ending code = %d\n", WEXITSTATUS(status));
        }

    }
    free(arguments);
    exit(0);
}