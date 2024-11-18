#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]){

    if (argc < 2) {
        fprintf(stderr, "Error");
        return 1;
    }   

    pid_t pid = fork();

    if (pid < 0) {

        perror("fork error");
        exit(1);
    }

    else if ( pid == 0) {

        execvp(argv[1], &argv[1]);
        perror("execvp failed");
        exit(1);
    }

     else { 

        printf("This is the parent process.\n");

        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Child process exit code %d\n", WEXITSTATUS(status));

        } else {
            printf("The child process exited with an error\n");
        }
    }
    return 0;
}








