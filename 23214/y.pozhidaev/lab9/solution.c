#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <wait.h>

int main(int argv, char *argc[]) {
    if (argv == 1) {
	perror("Nothing to cat");
        exit(1);
    }
    pid_t child1, pid;
    int status;
    child1 = fork();
    if (child1 == 0) {
        execl("/bin/cat", "cat", argc[1], (char *) 0);
	perror("Execl error");
	exit(2);
    } else if (child1 == -1) {
        perror("fork finished unsuccessfully");
        exit(3);
    } else {
        pid = wait(&status);

        if (pid == -1) {
            perror("wait finished unsuccessfully");
        } else if (pid == child1) {
            printf("Program is finished successfully\n");
        } else {
            perror("Wrong child");
        }
    }
    exit(0);
}

