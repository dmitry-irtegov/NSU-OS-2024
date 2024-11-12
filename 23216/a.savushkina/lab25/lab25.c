#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

#define BUFSIZE 255

int main() {
    int pipefd[2];

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

        char text[BUFSIZE] = "text for upper LOL\n";

        if (close(pipefd[0]) == -1) {
            perror("error in close");
            exit(EXIT_FAILURE);
        }

        if (write(pipefd[1], text, BUFSIZE) == -1) {
            perror("error in write");
            exit(EXIT_FAILURE);
        }

        if (close(pipefd[1]) == -1) {
            perror("error in close");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);

    default:

        char buf[BUFSIZE] = {0};

        if (close(pipefd[1]) == -1) {
            perror("error in close");
            exit(EXIT_FAILURE);
        }
        ssize_t lol;
        while (lol = read(pipefd[0], &buf, BUFSIZE)){
            if (lol == -1) {
                perror("error in read");
                exit(EXIT_FAILURE);
            }

            for (size_t i = 0; i < strlen(buf); i++) {
                buf[i] = toupper(buf[i]);
            }

            if (write(fileno(stdin), &buf, sizeof(buf)) == -1) {
                perror("error in write");
                exit(EXIT_FAILURE);
            }
        }
        if (lol == -1) {
            perror("error in read");
            exit(EXIT_FAILURE);
        }

        if (close(pipefd[0]) == -1) {
            perror("error in close");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
}