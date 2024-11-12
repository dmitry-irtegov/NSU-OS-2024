#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

#define BUFSIZE 5

int main() {
    int pipefd[2];
    char text[20] = "text for upper LOL\n";
    char buf[BUFSIZE] = {0};

    int pipe_status = pipe(pipefd);

    if (pipe_status == -1) {
        perror("error in pipe");
        exit(EXIT_FAILURE);
    }

    int fork_process = fork();
    switch (fork_process) {
    case -1:
        perror("error in fork");
        exit(EXIT_FAILURE);
    case 0:
        if (close(pipefd[0]) == -1) {
            perror("error in close");
            exit(EXIT_FAILURE);
        }

        if (write(pipefd[1], text, strlen(text)) == -1) {
            perror("error in write");
            exit(EXIT_FAILURE);
        }

        if (close(pipefd[1]) == -1) {
            perror("error in close");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);

    default:
        if (close(pipefd[1]) == -1) {
            perror("error in close");
            exit(EXIT_FAILURE);
        }

        ssize_t lol;
        ssize_t i;
        while ((lol = read(pipefd[0], buf, BUFSIZE - 1)) > 0) {
            if (lol == -1) {
                perror("error in read");
                exit(EXIT_FAILURE);
            }

            for (i = 0; i < lol; i++) {
                buf[i] = toupper(buf[i]);
            }

            if (write(fileno(stdout), buf, lol) == -1) {
                perror("error in write");
                exit(EXIT_FAILURE);
            }

            for (i = 0; i < lol; i++) {
                if (buf[i] == '\n') {
                    if (close(pipefd[0]) == -1) {
                        perror("error in close");
                        exit(EXIT_FAILURE);
                    }
                    exit(EXIT_SUCCESS);
                }
            }

            memset(buf, 0, BUFSIZE);
        }

        if (lol == -1) {
            perror("error in read");
            exit(EXIT_FAILURE);
        }
    }
}