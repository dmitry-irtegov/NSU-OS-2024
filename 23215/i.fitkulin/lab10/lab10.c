#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2)
    {
        fprintf(stderr, "Not enough arguments\n");
        exit(EXIT_FAILURE);
    } 

    pid_t pid = fork();
    int status;

    switch (pid) {
        case (0):
            if (execvp(argv[1], &argv[1]) == -1) {
                perror("Execvp() fail");
                exit(EXIT_FAILURE);
            }
        case (-1):
            perror("Fork() fail");
            exit(EXIT_FAILURE);
        default:
            if (waitpid(pid, &status, 0) == -1) {
                perror("Waitpid() fail");
                exit(EXIT_FAILURE);
            } 

            if (WIFEXITED(status)) {
                printf("proccess is over, status is %d\n", WEXITSTATUS(status));
            }       
    }
    
    exit(EXIT_SUCCESS);
}
