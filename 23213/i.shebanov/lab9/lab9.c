#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <stdint.h>

int main(void)
{
    pid_t pid;

    pid = fork();
    switch (pid) {
    case -1:
        perror("Failed to fork");
        exit(EXIT_FAILURE);
    case 0:
    if(execl("/bin/cat", "cat", "large_file.txt", NULL) == -1){
            perror("Failed to execute cat");
        exit(EXIT_FAILURE);		
    }
    puts("Child exiting.");
        exit(EXIT_SUCCESS);
    default:
    if (wait(NULL) == -1){
        perror("Child process hasn't finished successfuly");
        exit(EXIT_FAILURE);
    }
        puts("Cat has executed successfuly");
        exit(EXIT_SUCCESS);
    }
}
