#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
    pid_t fork_process, wait_child_process;
    int wstatus;
    
    fork_process = fork();

    switch (fork_process)
    {
    case -1:
        perror("error in forking process.");
        exit(EXIT_FAILURE);
    case 0:
        printf("Child process is existing\n");
        int cat_do = execlp("cat", "cat", argv[1], NULL);
        if (cat_do == -1)
        {
            perror("execlp error");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    default:
        wait_child_process = wait(&wstatus);
        if (wait_child_process == -1)
        {
            perror("error in waitpid.");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(wstatus))
        {
            printf("s\n");
        }
        break;
    }

    return 0;
}