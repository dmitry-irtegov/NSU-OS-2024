#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

#define BATCH_SIZE 1024

int main() {
    int pipefd[2];
    pid_t subprocess_id;
    char buffer[BATCH_SIZE];
    int wait_status;

    if (pipe(pipefd) == -1) {
        perror("pipe error");
        exit(1);
    }
    subprocess_id = fork();
    if (subprocess_id == -1) {
        perror("fork error");
        exit(1);
    }
    if (subprocess_id == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execlp("cat", "cat", "input.txt", NULL);
        perror("execlp error");
        exit(1);
    } else {
        close(pipefd[1]);
        while (read(pipefd[0], buffer, BATCH_SIZE) > 0) {
            for (int i = 0; i < BATCH_SIZE && buffer[i] != '\0'; i++) {
                buffer[i] = toupper(buffer[i]);
            }
            printf("%s", buffer);
        }
        close(pipefd[0]);
        if (wait(&wait_status) == -1) {
            perror("wait error");
            exit(1);
        }
    }
    exit(0);
}
