#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[])
{
    if (argc < 2){
        perror("Number of arguments < 2");
        exit(1);
    }
    
    pid_t pid;
    pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        exit(1);

    } else if (pid == 0){

        execvp(argv[1], &argv[1]);
        perror("exec failed"); 
        exit(1); 

    } else {

        int status;

        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
        }

        if(WIFSIGNALED(status)){

            int signal = WTERMSIG(status);
            printf("child terminated by signal %d\n", signal);

        } else if(WIFEXITED(status)){

            int code = WEXITSTATUS(status);
            printf("child terminated with code %d\n", code);

        }
    }
    exit(0);
}