#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 100

int main() {
    int fd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];

    ssize_t bytes_read;

    if (pipe(fd) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    pid = fork();

    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        close(fd[0]);

        const char *text = "Hello, Child Process!";

        ssize_t total_written = 0;
        ssize_t bytes_left = strlen(text) + 1; 
        
        while (bytes_left > 0) {
            ssize_t written = write(fd[1], text + total_written, bytes_left);

            if (written == -1) {
                perror("write error");
                exit(EXIT_FAILURE);
            }

            total_written += written;
            bytes_left -= written;
        }

        close(fd[1]); 
        wait(NULL);   
    } else {
        close(fd[1]);  

        size_t total_read = 0;
        while ((bytes_read = read(fd[0], buffer + total_read, BUFFER_SIZE - total_read)) > 0) {
            total_read += bytes_read;
            if (buffer[total_read-1] == '\0') {
                break; 
            }
        }

        if (bytes_read == -1) {
            perror("read failed");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; buffer[i] != '\0'; i++) {
            buffer[i] = toupper((unsigned char)buffer[i]);
        }

        close(fd[0]);
        printf("%s\n", buffer);
    }

    return 0;
}
