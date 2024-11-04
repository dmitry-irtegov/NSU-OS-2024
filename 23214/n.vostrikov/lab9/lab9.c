#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {
    pid_t child = fork();
    if (child == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    if (child == 0) {
        execlp("cat", "cat", "file.txt", NULL);
        perror("execle failed");
        exit(EXIT_FAILURE);
    }
    else {
        if (waitpid(child, NULL, 0) == -1) {
            perror("waitpid failed");
            exit(EXIT_FAILURE);
        }
        printf("SUCCESS!!!");
        exit(EXIT_SUCCESS);
    }
}