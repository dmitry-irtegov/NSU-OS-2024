#include <wait.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h> 

int main(){
    puts("start");
    pid_t child = fork();
    siginfo_t info;

    switch (child){
        case -1:
            perror("failed to fork");
            exit(EXIT_FAILURE);
            break;
        case 0:
            execlp("cat", "cat", "file.txt", NULL);
            perror("failed to execlp");
            exit(EXIT_FAILURE);
            break;
        default:
            if (waitid(P_PID, child, &info, WEXITED) == -1){
                perror("failed to waitid");
                exit(EXIT_FAILURE);
            }
            puts("end of parent's work");
            break;

    }

    exit(EXIT_SUCCESS);
}