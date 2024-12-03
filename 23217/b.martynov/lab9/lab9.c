#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char** argv) {
    if (argc <= 1)
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
        argv[0] = "cat";
        argv[2] = NULL;
        execvp("cat", argv);

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
        printf("Last line\n");
    }

    return 0;
}
