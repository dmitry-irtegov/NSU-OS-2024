#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(){

    pid_t pid = fork();

    if (pid < 0) {

        perorr("fork error");
        exit(1);
    }

    else if ( pid == 0) {

        execlp("cat", "cat", "longfile.txt", NULL); 
        perror("execlp failed");
        exit(1);
    }

     else { 

        printf("This is the parent process.\n");
        wait(NULL); 
        printf("child process executed");
    }
    return 0;
}