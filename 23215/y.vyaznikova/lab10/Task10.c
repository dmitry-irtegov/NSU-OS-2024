#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Insufficient number of arguments\n");
        exit(1);
    }

    pid_t subprocess_id = fork();

    if (subprocess_id == -1) {
        perror("Failed to create subprocess");
        exit(1);
    } else if (subprocess_id == 0) {
        if (execvp(argv[1], argv + 1) == -1) {
            perror("Failed to execute command");
            exit(2);
        }
    } else {
        int status;
        if (wait(&status) == -1) {
            perror("Failed to wait for the subprocess to finish");
            exit(3);
        }

        if (WIFEXITED(status)) {
            printf("Child process exited with status: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child process terminated by signal: %d\n", WTERMSIG(status));
        }
    }

    exit(0);
}