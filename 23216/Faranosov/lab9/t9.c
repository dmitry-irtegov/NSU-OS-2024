#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>


int main(int argv, char* argc[]){
    if (argv == 1){
        printf("No file to cat");
        exit(1);
    }

    pid_t child_id, finished_process;
    int status;

    child_id = fork();

    switch (child_id) {
    case 0:
        execl("/bin/cat", "cat", argc[1], (char*)0);

        perror("execl error");
        exit(1);
        break;

    case -1:
        perror("fork error");
        exit(1);
        break;

    default:
        finished_process = wait(&status);

        if (finished_process == child_id) {
            printf("Child terminated: child_id = %d\n", child_id);
        }
        else if (finished_process >= 0) {
            printf("fp != pid");
        }
        else if (finished_process == -1) {
            perror("Wait error");
            exit(1);
        }
    }

    exit(0);
}