#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>

int main(int argc, char** argv){
    if (argc > 2 || argc < 2){
        perror("Error: must be only 1 argument: ./lab9 <file>");
        exit(EXIT_FAILURE);
    }
    printf("Child process starded!\n");
    pid_t child_process = fork();
    if (child_process == -1){
        perror("Error: Cant`t creat child process!");
        exit(EXIT_FAILURE);
    }
    else if(child_process == 0){
        execlp("cat", "cat", argv[1], NULL);
        perror("Error: can`t use cat!");
        exit(EXIT_FAILURE);
    }
    else{
        int status;
        if (wait(&status) == -1){
            perror("Error: failed to wait!");
            exit(EXIT_FAILURE);
        }
        printf("\nPerent process ended!\n");
        exit(EXIT_SUCCESS);
    }
}
