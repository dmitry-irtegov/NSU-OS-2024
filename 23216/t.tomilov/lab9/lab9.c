#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char** argv){
    if (!(argc == 2)){
        perror("ERROR: wrong format of start! Try ./lab9 <file>");
        exit(EXIT_FAILURE);
    }
    int stat;
    printf("Child process started!\n");
    pid_t child_process = fork();
    if (child_process == -1){
        perror("ERROR: failed to fork!");
        exit(EXIT_FAILURE);
    }
    else if (child_process == 0){
        execlp("cat", "cat", argv[1], NULL);
        perror("ERROR: failed to execlp!");
        exit(EXIT_FAILURE);
    }
    else{
        if (waitpid(child_process, &stat, 0) == -1){
            perror("ERROR: failed to waitid!");
            exit(EXIT_FAILURE);
        }
        printf("\nPerent process ended!\n");
    }
    exit(EXIT_SUCCESS);
}