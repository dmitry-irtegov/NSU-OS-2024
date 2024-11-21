#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;
    const char *filename = "testfile.txt";
    pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        execlp("cat", "cat", filename, (char *)NULL);
        perror("execlp failed");
        exit(EXIT_FAILURE);
    } else {
        wait(NULL);
        printf("subprocess ended.\n");
    }
    return 0;
}