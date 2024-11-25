#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork error");
        return 1;
    }

    if (pid == 0)
    {
        if (execvp(argv[1], &argv[1]) == -1)
        {
            perror("execvp error");
            return 1;
        }
    }
    else
    { 
        int status;
        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid failed");
            return 1;
        }

        if (WIFEXITED(status))
        {
            printf("Child process exited with code %d\n", WEXITSTATUS(status));
        }
        else
        {
            printf("Child process did not exit normally\n");
        }
    }

    return 0;
}