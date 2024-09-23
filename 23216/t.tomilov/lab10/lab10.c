#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>

int main(int argc, char** argv){
    printf("Child process starded!\n");
    pid_t child_process = fork();
    if (child_process == -1){
        perror("Error: Cant`t creat child process!");
        exit(EXIT_FAILURE);
    }
    else if(child_process == 0){
        execvp(argv[1], &argv[1]);
        perror("Error: failed to execvp!");
        exit(EXIT_FAILURE);
    }
    else{
        int status;
        if (waitpid(child_process, &status, 0) == -1){
            perror("Error: failed to wait!");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(status) == 1){
            printf("Process ended with code %d\n", WEXITSTATUS(status));
        }
        else{
            printf("Process killed with code %d\n", WTERMSIG(status));
        }
        exit(EXIT_SUCCESS);
    }
}