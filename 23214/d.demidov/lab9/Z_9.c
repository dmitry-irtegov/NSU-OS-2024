#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {
    int process_id = getpid();
    int parent_process_id = getppid();
    printf("process id: %d parent process id: %d\n", process_id, parent_process_id);
    pid_t subprocess_id = fork();
    process_id = getpid();
    parent_process_id = getppid();
    printf("process id: %d parent process id: %d\n", process_id, parent_process_id);
    printf("subprocess_id = %d\n", subprocess_id);
    if (subprocess_id == -1) {
        perror("error creating subprocess");
        exit(1);
    } else {
        if (subprocess_id == 0) {
            execlp("cat", "cat", "input.txt", NULL);
            perror("execlp error");
            exit(1);
        } else {
            if (wait(NULL)) == -1) {
                perror("wait error");
            } else {
                printf("wait success \n");
                exit(0);
            }
        }
    }
}
