#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Not all arguments\n");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork fail");
        return 1;
    }

    if (pid == 0) {
        if (execvp(argv[1], &argv[1]) == -1) {
            perror("execvp fail");
            return 1;
        }
    }
    else {
        printf("Parent process is waiting for the child finish\n");
        int status;
        if (waitpid(pid, &status, 0) != -1) {
            if (WIFEXITED(status)) {
                printf("Child process has finished with status %d\n", WEXITSTATUS(status));
            } 
            else if (WIFSIGNALED(status)) {
                printf("Child process terminated by signal, signal id = %d\n", WTERMSIG(status));
            }
        } 
        else {
            perror("waitpid fail");
        }
    }

    return 0;
}

