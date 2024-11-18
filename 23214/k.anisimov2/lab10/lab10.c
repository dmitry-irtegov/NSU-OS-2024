#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "No arguments");
        return EXIT_FAILURE;
    }

    int status;
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("Error while calling fork");
        return EXIT_FAILURE;
    }
    else if (pid == 0)
    {
        execvp(argv[1], &argv[1]);
        perror("Error while executing execvp");
        exit(EXIT_FAILURE);
    }
    else
    {
        if (waitpid(pid, &status, 0) == -1)
        {
            perror("Error while calling waitpid");
            return EXIT_FAILURE;
        }

        if (WIFEXITED(status))
        {
            printf("Completion Code: %d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("The process ended due to the signal: %d\n", WTERMSIG(status));
        }
        return EXIT_SUCCESS;
    }
}