#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    if (argc < 2){
        fprintf(stderr, "not enough arguments\n");
        exit(-1);
    }
    int status;
    pid_t pid = fork();  
    switch (pid)
    {
    case -1:
        perror("fork failed");
        exit(-1);
    case 0:
        execlp("cat", "cat", argv[1], NULL);
        perror("execlp failed");
        exit(-1);
    default:
        printf("success\n");
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
            exit(-1);
        }
        printf("\nfork exited\n");
        exit(0);
    }
}
