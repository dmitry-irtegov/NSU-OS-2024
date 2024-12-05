#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#define BUFFER_SIZE 256

int main() {
    int pipe_fd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];

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
        ssize_t n;
        while ((n = read(pipe_fd[0], buffer, BUFFER_SIZE)) > 0) {
            for (ssize_t i = 0; i < n; i++) {
                buffer[i] = toupper(buffer[i]);
            }
            write(STDOUT_FILENO, buffer, n);
        }
        close(pipe_fd[0]);
    } else {
        close(pipe_fd[0]); 
        const char *message = "Hello, Pipe Communication!\n";
        write(pipe_fd[1], message, strlen(message));
        close(pipe_fd[1]);
        wait(NULL);       
    }

    return 0;
}
