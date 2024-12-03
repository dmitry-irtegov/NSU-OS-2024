#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        execlp("cat", "cat", "longfile.txt", (char *)NULL);
        perror("execlp failed");
        exit(EXIT_FAILURE);
    } else {
        printf("Parent process: waiting for the child to complete.\n");
        wait(NULL);
        printf("\nChild process completed.\n");
    }

    return 0;
}

