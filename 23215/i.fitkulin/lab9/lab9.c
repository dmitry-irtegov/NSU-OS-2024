#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

int main() {
    pid_t pid = fork();
    int status;

    switch (pid) {
        case (0):
            if (execlp("cat", "cat", "file.txt", NULL) == -1); {
                perror("Execlp() fail");
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
            printf("SUCCESS\n");
    } 
    
    exit(EXIT_SUCCESS);
}