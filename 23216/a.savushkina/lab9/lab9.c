#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    pid_t fork_process, wait_child_process;
    int wstatus = 0;
    if (!argv[1]) {
        printf("no file for cat\n");
        exit(EXIT_FAILURE);
    }
    fork_process = fork();

    switch (fork_process){
    case -1:
        perror("error in forking process.");
        exit(EXIT_FAILURE);
    case 0:
        printf("Child process is existing\n");
        int cat_do = execlp("cat", "cat", argv[1], NULL);
        perror("execlp error");
        exit(EXIT_FAILURE);
    default:
        wait_child_process = waitpid(fork_process, &wstatus, 0);
        if (wait_child_process == -1) {
            perror("error in waitpid.");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(wstatus)){
            int exit_code = WEXITSTATUS(wstatus);
            if (exit_code == EXIT_SUCCESS) {
                printf("Child process exited successfully\n");
            } else {
                printf("Child process exit was unsuccessful with code: %d\n", exit_code);
            }
        } else {
            printf("Child process did not terminate normally\n");
        }
        break;
    }

    return 0;
}