#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

int main() {
    int fd[2], status;
    ssize_t size;
    if (pipe(fd) == -1) {
        perror("Failed to create pipe");
        exit(-1);
    }
    pid_t fork_pid = fork();
    if (fork_pid == -1) {
        perror("Failed to create child by fork");
        exit(-1);
    } else if (fork_pid == 0) {
        char text[BUFSIZ];
        if (close(fd[1]) == -1) {
            perror("Failed to close write pipe end in child");
            exit(-1);
        }
        while ((size = read(fd[0], text, BUFSIZ)) > 0) {
            for (int i = 0; i < size; i++) {
                text[i] = toupper(text[i]);
            }
            printf("%s\n", text);
        }
        if (size == -1) {
            perror("Failed to read message in child");
            exit(-1);
        }
        if (close(fd[0]) == -1) {
            perror("Failed to close read pipe end in child");
            exit(-1);
        }
        exit(0);
    } else {
        char msg[BUFSIZ] = "abCDefGHIjKlMnoprstuvWXYZ";
        if (close(fd[0]) == -1){
            perror("Failed to close read pipe end");
        }
        if (write(fd[1], msg, strlen(msg) + 1) == -1) {
            perror("Failed to write message");
        }
        if (close(fd[1]) == -1){
            perror("Failed to close write pipe end");
        }
        pid_t wait_pid = waitpid(fork_pid, &status, 0);
        if (wait_pid == -1) {
            perror("Error with waiting the child process");
            exit(-1);
        }
        if (WIFEXITED(status)) {
            printf("\nchild pid = %d \nEverything success\n", wait_pid);
        }
    }
    exit(0);
}
