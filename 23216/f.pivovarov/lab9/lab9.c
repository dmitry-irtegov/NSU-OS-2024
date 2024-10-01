#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main() {

    printf("Try to fork()\n");
    // Forks the current process
    pid_t process_id = fork();

    if (process_id == -1) {
        perror("Can not fork()");
        exit(-1);
    }
    else if (process_id == 0) {
        // Forked process code
        printf("Inside forked\n");
        int exec_res = execlp("cat", "cat", "too_large_file", (char *)NULL);
        
        if (exec_res == -1) {
            perror("An error in `exec(cat, too_large_file)`");
            exit(-1);
        }

        exit(0);
    }

    // Parent process code
    int child_status;
    pid_t wait_status = waitpid(process_id, &child_status, 0);

    if (wait_status == -1) {
        perror("Occured an error");
        exit(-1);
    }

    printf("Return to parent.\nChild has finished with success!!\n");

    exit(0);
}
