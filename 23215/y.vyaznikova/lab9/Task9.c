#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    pid_t subprocess_id = fork();

    if (subprocess_id == -1)
    {
        perror("Failed to create subprocess");
        exit(1);
    }
    else
    {
        if (subprocess_id == 0)
        {
            if (execlp("cat", "cat", "Task9.c", NULL) == -1)
            {
                perror("Failed to display the contents of the file \"Task9.c\"");
                exit(1);
            }
        }
        else
        {
            printf("The result of executing the cat file \"Task9.c\":\n");

            if (wait(NULL) == -1)
            {
                perror("Failed to wait for the subprocess to finish");
                exit(1);
            }

            printf("\n\nThe parent's process message:\nThe parent process has completed after the subprocess finished\n");
        }
    }

    exit(0);
}