#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
int main(int argc, char* argv[])
{
    if(argc < 2){
        perror("missing command");
        exit(1);
    }
    int pid;
    if((pid = fork()) == 0){
        printf("child id = %d\n", getpid());
        execvp(argv[1], argv + 1);
        perror("exec error");
        exit(2);
    }
    else if(pid > 0) {
        int status = 0;
        printf("parent is waiting\n");
        int ret = wait(&status);
        printf("status = %d\n", status);
        if(ret == -1){
            perror("wait error");
            exit(3);
        }
        else{
            if(WIFEXITED(status)){
                printf("exit code = %d\n", WEXITSTATUS(status));
            }
            else if(WIFSIGNALED(status)){
                printf("signal code = %d\n", WTERMSIG(status));
            }
            else{
                perror("unexpected error");
                exit(4);
            }
        }
    }
    else{
        perror("fork error");
        exit(5);
    }
    exit(0);
}