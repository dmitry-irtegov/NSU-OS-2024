#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main() {

    printf("Try to fork()\n");
    // Forks the current process
    pid_t process_id = fork();

    switch (process_id) {
        case -1:
            perror("Can't fork()");
            exit(EXIT_FAILURE);
        case 0:
            // Forked process code
            printf("Inside forked\n");
            execlp("cat", "cat", "too_large_file", (char *)NULL);

            perror("Can't exec cat");
            exit(EXIT_FAILURE);
        default:
            // Parent process code
            int child_status;
            pid_t wait_status = waitpid(process_id, &child_status, 0);

            if (wait_status == EXIT_FAILURE) {
                perror("Occured an error");
                exit(EXIT_FAILURE);
            }

            printf("Return to parent.\nChild has finished with success!!\n");

            exit(EXIT_SUCCESS);
        }
}
