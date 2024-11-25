#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    pid_t pid;
    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        execlp("cat", "cat", "long_file.txt", (char *)NULL);
        perror("execlp failed");
        exit(1);
    } else {
        printf("Parent process: this is a message from the parent.\n");
       
        wait(NULL);
        
        printf("Parent process: child has finished, printing final message.\n");
    }

    return 0;
}
