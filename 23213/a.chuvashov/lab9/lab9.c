#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Wrong amount of arguments\n");
        return -1;
    }

    pid_t pid;
    if ((pid = fork()) > 0) {
        wait(NULL);
        printf("This is parent process\n");
    } else if (pid == 0) {
        execl("cat", "cat", argv[1], NULL);
        perror("Couldn`t exec");
        return -1;

    } else {
        perror("Couldn`t fork");
        return -1;
    }
    return 0;
}
