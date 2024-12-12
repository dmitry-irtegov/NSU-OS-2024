#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    if (argc != 2) {
        if (argc > 0) {
            printf("Usage: %s <file>\n", argv[0]);
        } else {
            printf("Usage: lab9 <file>\n");
        }
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1){
        perror("Can't fork");
        return 1;
    } else if (pid == 0){
        execlp("cat", "cat", argv[1], NULL);
        perror("Can't exec cat");
        return 1;
    } else {
        wait(NULL);
        printf("\nChild exited\n");
    }

    return 0;
}
