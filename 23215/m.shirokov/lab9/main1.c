#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid;
    const char *filename = "largefile.txt";
    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        execlp("cat", "cat", filename, (char *)NULL);
        perror("execlp failed");
        exit(EXIT_FAILURE);
    } else {
        printf("Parent process is running.\n");
    }

    return 0;
}