#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>

#define BUFFERSIZE 20

int main() {

    int fd[2];
    pid_t pid;

    if (pipe(fd) != 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    if ((pid = fork()) < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {                 //child
        if (close(fd[1]) != 0) {
            perror("close fd[1] in child process");
            exit(EXIT_FAILURE);
        }

        char buff[BUFFERSIZE];
        int bytes_read;
        while ((bytes_read = read(fd[0], buff, BUFFERSIZE)) > 0) {
            for (int i = 0; i < bytes_read; i++) {
                buff[i] = toupper(buff[i]);
            }
            write(1, buff, bytes_read);
        }

        if (close(fd[0]) != 0) {
            perror("close fd[0] in child process");
            exit(EXIT_FAILURE);
        }
    } else {                        //parent
        if (close(fd[0]) != 0) {
            perror("close fd[0] in parent process");
            exit(EXIT_FAILURE);
        }

        char str[] = "hElLo FrOm PaReNt PrOcEsS\n";
        write(fd[1], str, strlen(str));

        if (close(fd[1]) != 0) {
            perror("close fd[1] in parent process");
            exit(EXIT_FAILURE);
        }

        wait(NULL);
    }

    exit(EXIT_SUCCESS);
}
