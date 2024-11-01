#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/wait.h>

#define BUFSIZE 1024

void processReader(int read_fd) {
    char buffer[BUFSIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(read_fd, buffer, BUFSIZE)) > 0) {
        for (size_t i = 0; i < bytes_read; ++i) {
            printf("%c", toupper(buffer[i]));
        }
    }
    if (bytes_read == -1) {
        perror("fail reading from pipe");
        close(read_fd);
        exit(EXIT_FAILURE);
    }
    printf("\n");
    close(read_fd);
}

void processWriter(int write_fd) {
    char buffer[BUFSIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(STDIN_FILENO, buffer, BUFSIZE)) > 0) {
        if (write(write_fd, buffer, bytes_read) == -1) {
            perror("fail writing to pipe");
            close(write_fd);
            exit(EXIT_FAILURE);
        }
    }
    if (bytes_read == -1) {
        perror("read error");
        close(write_fd);
        exit(EXIT_FAILURE);
    }
    close(write_fd);
}

int main() {
    int fildes[2];
    if (pipe(fildes) == -1) {
        perror("fail creating pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fail creating process");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        close(fildes[1]);
        processReader(fildes[0]);
    } else {
        close(fildes[0]);
        processWriter(fildes[1]);
        int status;

        if (waitpid(pid, &status, 0) == -1) {
            perror("fail waiting for child process");
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}
