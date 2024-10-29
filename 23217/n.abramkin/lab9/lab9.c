#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork(); // Create a child process
    char *filename = "largefile.txt"; 

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }
    else if (pid == 0) { // Child process
        printf("Child process: Executing 'cat' on a long file.\n");
        
        execlp("cat", "cat", filename, (char *)NULL);
        
        perror("execlp failed");
        exit(1);
    }
    else { // Parent process
        printf("Parent process: My child process has PID %d.\n", pid);

        // Wait for the child process to complete
        waitpid(pid, NULL, 0);

        printf("Parent process: Child process completed.\n");
    }

    return 0;
}
