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
        execvp(argv[1], argv + 1);
        perror("If cat was executed then error wouldn't be displayed");
        exit(1);
    } else {
        
        int status;
        pid_t new_pid = wait(&status);

        if (new_pid == -1) {
            perror("Failure to get a exit pid status");
            exit(1);
        }

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Process was termenated by exit(2) with code: %d\n", exit_code);
        } else if (WIFSIGNALED(status)) {
            int signal_number = WTERMSIG(status);
            printf("Process was termenated by signal with number: %d\n", signal_number);
        }

    }

}
