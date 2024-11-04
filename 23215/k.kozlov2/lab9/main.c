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
        if (execlp("cat", "cat", argv[1], NULL) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else {
        if (wait(NULL) == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }
        printf("done!\n");
    }

    exit(EXIT_SUCCESS);
}
