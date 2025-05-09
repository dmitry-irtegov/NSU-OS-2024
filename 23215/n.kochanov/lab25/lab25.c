#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#define BUFFER_SIZE 256

int main() {
    int pipe_fd[2]; // pipe_fd[0] для чтения, pipe_fd[1] для записи
    pid_t pid;
    char buffer[BUFFER_SIZE];

    if (pipe(pipe_fd) == -1) {
        perror("pipe creation failed");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        close(pipe_fd[1]);

        ssize_t bytes_read;
        while ((bytes_read = read(pipe_fd[0], buffer, BUFFER_SIZE)) > 0) {
            for (ssize_t i = 0; i < bytes_read; i++) {
                buffer[i] = toupper(buffer[i]);
            }
            write(STDOUT_FILENO, buffer, bytes_read);
        }

        if (bytes_read == -1) {
            perror("read from pipe failed");
        }

        close(pipe_fd[0]);
        exit(EXIT_SUCCESS);

    } else { // род процесс
        close(pipe_fd[0]);

        printf("enter text (CTRL+D to finish):\n");
        ssize_t bytes_read;
        while ((bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) {
            if (write(pipe_fd[1], buffer, bytes_read) == -1) {
                perror("write to pipe failed");
                break;
            }
        }

        close(pipe_fd[1]);
        wait(NULL);
        exit(EXIT_SUCCESS);
    }
}