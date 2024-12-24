#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int status;
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
    }

    if (pid == 0) {
        execlp("cat", "cat", "long_file.txt", NULL);
    }

    else {
        printf("end subprocess pid %d with status %d\n", wait(&status), WEXITSTATUS(status));
        printf("Hello world!\n");
    }

    return 1;
}
