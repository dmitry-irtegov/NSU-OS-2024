#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>


int main() {
    int pipe_fd[2];
    pid_t pid;
    char buf[BUFSIZ];
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        close(pipe_fd[1]);
        ssize_t bytes_recieved;
        while ((bytes_recieved = read(pipe_fd[0], buf, BUFSIZ)) > 0) {
            for (int i = 0; buf[i]; i++) {
                buf[i] = toupper(buf[i]);
            }
            write(1, buf, bytes_recieved);
        }
        close(pipe_fd[0]);
        exit(EXIT_SUCCESS);
    } else {
        close(pipe_fd[0]);
        const char *string = "Hello, World!\n";
        if (write(pipe_fd[1], string, strlen(string) + 1) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        close(pipe_fd[1]);
        if (wait(NULL) == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }
    }
    return EXIT_SUCCESS;
}