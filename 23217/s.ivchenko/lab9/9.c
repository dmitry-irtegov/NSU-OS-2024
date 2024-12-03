#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int status;
    pid_t pid = fork();

    printf("Start, pid: %d\n", pid);
    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        execlp("cat", "custom_cat", "file.txt", (char *)NULL);
        // printf("test\n");
        perror("execlp failed");
        return 1;
    }
    waitpid(pid, &status, 0);
    // wait(&status);
    printf("Parent process: Printing some text.\n");
    return 0;
}

//The exec family of functions replaces the current running 
//process with a new process. It comes under the header file unistd.h.
//execlp searches for the file (1st argument of execlp) 
//in those directories pointed by PATH.
