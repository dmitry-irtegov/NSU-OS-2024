#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid;
    const char *filename = "largefile.txt";
    int status;
    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        execlp("cat", "cat", filename, (char *)NULL);
        perror("execlp failed"); 
        exit(EXIT_FAILURE);
    } else {
        printf("Parent is waiting for the child process to complete...\n");
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
            exit(EXIT_FAILURE);
        }
        printf("Child process completed. Parent is now exiting.\n");
    }
    return 0;
}
