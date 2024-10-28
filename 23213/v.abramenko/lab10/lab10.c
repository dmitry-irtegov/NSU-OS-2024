#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char **argv) {

    if (argc < 2){
        fprintf(stderr, "not enough arguments\n");
        exit(-1);
    }

    int status;
    pid_t pid = fork();
    switch (pid)
    {
    case -1:
        perror("fork failed");
        exit(-1);
    case 0:
        execvp(argv[1], &argv[1]);
        perror("execvp failed");
        exit(-1);
    default:
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
            exit(-1);
        }
        if (WIFEXITED(status)){
            printf("exited with status = %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("terminated by signal = %d\n", WTERMSIG(status));
        }
        exit(0);
    }
}
