#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

int main() {
    int pipefd[2];
    char buf;

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

        if (write(pipefd[1], "text for upper LOL\n", 20) == -1) {
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

        if (wait(NULL) == -1) {
            perror("error in wait");
            exit(EXIT_FAILURE);
        }
        ssize_t lol;
        while ((lol = read(pipefd[0], &buf, 1)) > 0) {
            buf = toupper(buf);
            if (write(fileno(stdin), &buf, 1) == -1) {
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