#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];
    pid_t pid;
    char msg[BUFSIZ];
    if (fgets(msg, sizeof(msg), stdin) == NULL) {
        perror("Error reading input");
        exit(EXIT_FAILURE);
    }

    size_t len = strlen(msg);

    if (len == 0) {
        fprintf(stderr, "Input string is empty.\n");
        exit(EXIT_FAILURE);
    }

    if (msg[len - 1] == '\n') {
        msg[len - 1] = '\0';
        len--; 
    }

    int msgSize = len + 1; 
    
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    pid = fork();

    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        close(pipefd[1]);

        char getMsg[msgSize];
        if (read(pipefd[0], getMsg, msgSize) == -1) {
            perror("read failed");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < msgSize; i++) {
            printf("%c", toupper(getMsg[i]));
        }
        printf("\n");
        close(pipefd[0]);

    } else {
        close(pipefd[0]);

        if (write(pipefd[1], msg, msgSize) == -1) {
            perror("write failed");
            exit(EXIT_FAILURE);
        }
        close(pipefd[1]);

        wait(NULL);
    }

    return 0;
}

