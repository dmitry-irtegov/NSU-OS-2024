#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>

int main(int argc, char *argv[]){
    if (argc < 2){
        perror("incorrect number of arguments\n");
        exit(EXIT_FAILURE);
    }
    int status;
    pid_t pid, child;

    pid = fork();
    switch (pid){
        //error handling
        case -1: 
            perror("failed to fork the process\n");
            exit(EXIT_FAILURE);
        //child process
        case 0 :
            execvp(argv[1], &argv[1]);
            //if execvp returned failure
            perror("failed to execvp\n");
            exit(EXIT_FAILURE);

        default:
            child = wait(&status);
            if (child == -1) {
                perror("failed to wait\n");
                exit(EXIT_FAILURE);
            }
            if(WIFEXITED(status)){
                printf("proccess successfully ended with code %d\n", WEXITSTATUS(status));
            }
            else{
                printf("proccess killed with code %d\n", WTERMSIG(status));
            }
    }
    exit(EXIT_SUCCESS);
}
