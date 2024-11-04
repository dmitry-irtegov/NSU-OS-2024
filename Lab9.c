#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[]){
    if (argc != 2){
        perror("incorrect number of arguments");
        exit(EXIT_FAILURE);
    }
    int status;
    pid_t pid, child;
    pid_t parent = getpid();

    pid = fork();
    switch (pid){
        //error handling
        case -1: 
            perror("failed to fork the process");
            exit(EXIT_FAILURE);
        //child process
        case 0 :
            execlp("cat", "cat", argv[1], NULL);

            //if execl returned failure
            perror("failed to execl");
            exit(EXIT_FAILURE);

        default :
            child = wait(&status);

            if (child == -1) {
                perror("failed to wait");
                exit(EXIT_FAILURE);
            }
            printf("some text");
    }
    exit(EXIT_SUCCESS);
}