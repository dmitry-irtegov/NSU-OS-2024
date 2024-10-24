#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>


int main(int argc, char* argv[]){
    if (argc < 2){
        perror("Command is not found");
        exit(EXIT_FAILURE);
    }
    
    pid_t child = fork();
    int status;
    switch (child){
        case -1:
            perror("fork failed");
            break;
        case 0:
            execvp(argv[1], &argv[1]);
            perror("failed to execlp");
            exit(EXIT_FAILURE);
            break;
        default:
            if (waitpid(child, &status, 0) == -1){
                perror("failed to waitid");
                exit(EXIT_FAILURE);
            }

            if (WIFEXITED(status)){
                printf("process ended with code = %d\n", WEXITSTATUS(status));
            }
            else{
                printf("process was killed by signal with code = %d", WTERMSIG(status));
            }
            
            break;
    }

    exit(EXIT_SUCCESS);
}
