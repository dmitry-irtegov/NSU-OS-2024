#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Too few argv");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        if (execvp(argv[1], &argv[1]) == -1)
        {
            perror("Execution failed");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        int status;
        if (waitpid(pid, &status, 0) == -1)
        {
            perror("Waitpid failed");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status))
        {
            printf("\nProcess completed with exit code: %d\n", WEXITSTATUS(status));
        }
        else
        {
            fprintf(stderr, "Child process did not terminate normally.\n");
        }
    }

    return 0;
}