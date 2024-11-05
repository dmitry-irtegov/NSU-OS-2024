#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>


int main(int argc, char *argv[]) {
    if (argc < 2) {
        perror("Wrong usage");
        return EXIT_FAILURE;
    }

    pid_t pid;
    int status;
    if ((pid = fork()) == -1) {
        perror("Fork error");
        return EXIT_FAILURE;
    } else if (pid == 0) {
        char** arg_list = (char**)malloc(sizeof(char*) * (argc));
        if (!arg_list) {
            perror("Malloc error");
            return EXIT_FAILURE;
        }
        arg_list[0] = argv[1];
        for (int i = 1; i < argc - 1; ++i) {
            arg_list[i] = argv[i + 1];
        }
        arg_list[argc - 1] = NULL;
        if (execvp(argv[1], arg_list) == -1) {
            free(arg_list);
            perror("Execvp error");
            return EXIT_FAILURE;
        }
    } else {
        if (wait(&status) == -1) {
            perror("Wait error");
            return EXIT_FAILURE;
        }
        if (WIFEXITED(status)) {
            printf("%d\n", WEXITSTATUS(status));
        };
    }
    return EXIT_SUCCESS;
}
