#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

int main(int argc, char** argv){
    if (argc < 2){
        perror("ERROR: wrong format ot start! Try ./lab10 <process> <args_of_process>...");
        exit(EXIT_FAILURE);
    }
    pid_t child_process = fork();
    int stat;
    if (child_process == -1){
        perror("ERROR: failed to fork!");
        exit(EXIT_FAILURE);
    }
    else if (child_process == 0){
        execvp(argv[1], &argv[1]);
        perror("ERROR: faild to execvp");
        exit(EXIT_FAILURE);
    }
    else{
        if (waitpid(child_process, &stat, 0) == -1){
            perror("failed to waitid");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(stat)){
            printf("Process ended with code = %d\n", WEXITSTATUS(stat));
        }
        else{
            printf("Process killed with code = %d", WTERMSIG(stat));
        }
    }
}