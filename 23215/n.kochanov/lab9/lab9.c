#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0)
    {
        // Дочерний процесс выполняет cat
        execlp("cat", "cat", "file.txt", NULL);
        perror("exec failed");
        exit(1);
    }
    else
    {
        // Родительский процесс ждет завершения дочернего
        int status;
        waitpid(pid, &status, 0);
        printf("-------------------------\n");
        printf("Child process completed with status: %d\n", WEXITSTATUS(status));
        printf("This is the parent process.\n");
    }

    return 0;
}