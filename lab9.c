#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main()
{
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("\nerror : create child process\n");
        return 1;
    }

    if (pid == 0)
    {
        execlp("cat", "cat", "bigfile.txt", NULL);

        perror("\nerror : cat\n");

        return 1;
    }
    else
    {
        if (wait(NULL) != -1)
        {
            printf("\nchild proccess finished, pid: %d\n", pid);
        }
        else
        {
            perror("\nerror : child process finish\n");
        }
    }

    return 0;
}
