#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char** argv) {
    if (argc < 2)
    {
        printf("Not enough argument. At least one is needed.\n");
        exit(EXIT_FAILURE);
    }

    int status;
    pid_t child_pid = fork();

    if (child_pid == -1)
    {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0)
    {
        execvp(argv[1], argv + 1);

        perror("execvp() cat failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Parent process. Child pid: %d\n", child_pid);
        if (waitpid(child_pid, &status, 0) == -1)
        {
            perror("waitpid() failed");
            exit(EXIT_FAILURE);
        }
        
        if (WIFEXITED(status))
        {
            int exitStatus = WEXITSTATUS(status);
            printf("Process was done with code: %d\n", exit_status);
        } 
        else if (WIFSIGNALED(status))
        {
            int signal_number = WTERMSIG(status);
            printf("Process was terminated by signal with number: %d\n", signal_number);
        }
    }

    return 0;
}
