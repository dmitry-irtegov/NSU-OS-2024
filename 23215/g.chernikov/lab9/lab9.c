#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;
    pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        exit(1);
    } else if (pid == 0){

        execlp("cat","cat", "file.txt", NULL);
        perror("exec failed"); 
        exit(1); 

    } else {
        wait(NULL);
        printf("This is parent process");
    }

    exit(0);
}