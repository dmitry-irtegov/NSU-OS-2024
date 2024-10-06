#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <wait.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

#define BATCHSIZE 1024

int main(int argc, char* argv[]) {

    if (argc != 2) {
        fprintf(stderr, "wrong passing of arguments");
        return 1;
    }

    pid_t pid;
    int fd[2];
    char string_received[BATCHSIZE] = "";
    
    if (pipe(fd) == -1) {
        perror("pipe error");
        return 1;
    }

    pid = fork();

    if (pid == -1) {
        perror("fork error");
        close(fd[0]);
        close(fd[1]);
        return 1;

    } else if (pid == 0) { 
        close(fd[0]);

        int file_fd = open(argv[1], O_RDONLY);
        if (file_fd == -1) {
            perror("file open error");
            close(fd[1]);
            return 1;
        }

        ssize_t bytes_read;
        char buffer[BATCHSIZE];

        while ((bytes_read = read(file_fd, buffer, BATCHSIZE)) > 0) {
            if (write(fd[1], buffer, bytes_read) == -1) {
                perror("write error");
                close(file_fd);
                close(fd[1]);
                return 1;
            }
        }

        if (bytes_read == -1) {
            perror("file read error");
        }

        close(file_fd);
        close(fd[1]);
        return 0;
        
    } else {
        close(fd[1]); 
        ssize_t bytes_received = 0;

        while ((bytes_received = read(fd[0], string_received, BATCHSIZE)) > 0) {
            for (int i = 0; i < bytes_received; i++) {
                printf("%c", toupper(string_received[i]));
            }
        }
        if (bytes_received == -1) {
            perror("read error");
            close(fd[0]);
            return 1;
        }
        printf("\n");
        close(fd[0]);

        if (waitpid(pid, NULL, 0) == -1) {
            perror("waitpid error");
            return 1;
        }
        return 0;
    }
}
