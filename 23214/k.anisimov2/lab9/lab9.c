#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("Error while calling fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        execlp("cat", "cat", "long_file.txt", NULL);
        perror("Error while executing execlp");
        exit(EXIT_FAILURE);
    }
    else
    {
        if (waitpid(pid, NULL, 0) == -1)
        {
            perror("Error while calling waitpid");
            exit(EXIT_FAILURE);
        }
        printf("Child process terminated.\n");
        exit(EXIT_SUCCESS);
    }
}
