#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char *argv[]){
    
    pid_t pid;

    if (argc < 2){
        fprintf(stderr, "No arguments passed\n");
        return 1;
    }

    pid = fork();

    if (pid == -1){ 
        perror("fork error");
        return 1;

    } else if (pid == 0){
        execvp(argv[1], &argv[1]);
        perror("execvp error");
        return 1;

    } else {
        int wstatus;
        if (waitpid(pid, &wstatus, 0)== -1){
            perror("waitpid error");
            return 1;
        }
        if (WIFEXITED(wstatus)){
            printf("exited, status = %d\n", WEXITSTATUS(wstatus));
        } else if (WIFSIGNALED(wstatus)){
            printf("process terminated by signal, signal id = %d\n", WTERMSIG(wstatus));
        }
    } 

    return 0;
}
