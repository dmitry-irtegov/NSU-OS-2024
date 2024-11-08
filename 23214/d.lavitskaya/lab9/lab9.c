#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Correct usage: %s <file>\n", argv[0]);
        return 1;
    }

    pid_t child_pid = fork(); 

    if (child_pid == -1) {
        perror("fork failed");
        return 1;
    }

    if (child_pid == 0) {
        execlp("cat", "cat", argv[1], (char*) NULL);
        perror("exec failed");
        return 1;

    } else {
        int status;
        waitpid(child_pid, &status, 0); 

        if (WIFEXITED(status)) {
            printf("Parent process: child process exited with code %d\n", WEXITSTATUS(status));
        } else {
            printf("Parent process: child process exited with an error.\n");
        }
    }

    return 0;
}
