#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>

int main() {

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork fail");
        return 1;
    }

    if (pid == 0) {
        if (execlp("cat", "cat", "file.c", NULL) == -1) {
            perror("execlp fail");
            return 1;
        }
    } 
    else {
        printf("Parent process is waiting for the child finish\n");
        if (wait(NULL) != -1) {
            printf("Child process has finished\n");
        } 
        else {
            perror("wait fail");
        }
    }
    
    return 0;
}
