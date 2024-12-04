#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>


int main(int argc, char *argv[]) {

    if (argc < 2) {
        perror("Enter your arguments.\n");
        return 1;
    }
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    } else if (pid == 0) {
        execvp(argv[1], argv + 1);
        perror("Execvp failed");
        return 1;
    } else {
        int status;
        pid_t new_pid = wait(&status);

        if (new_pid == -1) {
            perror("Error with exit pid status");
            return 1;
        }

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Process was termenated by exit with code: %d\n", exit_code);
        } else if (WIFSIGNALED(status)) {
            int signal_number = WTERMSIG(status);
            printf("Process was termenated by signal with number: %d\n", signal_number);
        }

    }

}