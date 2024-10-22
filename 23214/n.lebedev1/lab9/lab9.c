#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

int main() {
    int status;
    pid_t current_pid = getpid();
    pid_t parent_pid = getppid();
    printf("Before fork:\n current pid = %d\n parent pid = %d\n", current_pid, parent_pid);
    pid_t fork_pid = fork();
    parent_pid = getppid();
    printf("After fork:\n fork pid = %d\n parent pid = %d\n", fork_pid, parent_pid);
    printf("\n");
    if (fork_pid == -1) {
        perror("Failed to create child by fork");
        exit(-1);
    }
    else if (fork_pid == 0) {
        if (execl("/bin/cat", "cat", "./file.txt", NULL) == -1) {
            perror("Failed to execute cat");
            exit(-1);
        }
    }
    pid_t wait_pid = waitpid(fork_pid, &status, 0);
    if (wait_pid == -1) {
        perror("Error with waiting the child process");
        exit(-1);
    }
    if (WIFEXITED(status)) {
        printf("\nchild pid = %d \nEverything success\n", wait_pid);
    }
    return 0;
}