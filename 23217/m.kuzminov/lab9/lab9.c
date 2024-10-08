#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>


int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "No filename\n");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Can't fork process");
        exit(1);
    }    

    if (pid == 0) {
        execlp("cat", "cat", argv[1], (char *) 0);
        perror("If cat was executed then error wouldn't be displayed");
        exit(1);
    } else {
        printf("First half of text\n");
        int status;
        pid_t new_pid = wait(&status);
        
        if (new_pid == -1) {
            perror("Failure to get a exit pid status");
            exit(1);
        }

        printf("\nSecond half of text\n");

    }

}
