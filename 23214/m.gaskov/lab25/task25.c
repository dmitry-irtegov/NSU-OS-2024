#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

int main() {
    int fd[2];
    char buffer[BUFFER_SIZE];
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid;
    if ((pid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        close(fd[1]);
        while (1) {
            ssize_t count = read(fd[0], buffer, BUFFER_SIZE - 1);
            if (count == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            } else if (count == 0) {
                break;
            }
            int i;
            for (i = 0; i < count; i++) {
                buffer[i] = toupper(buffer[i]);
            }
            buffer[count] = '\0';
            printf("%s", buffer);
        }
        close(fd[0]);
    } else {
        char* text = "HEllo world\n";
        int length = strlen(text);
        close(fd[0]);
        if (write(fd[1], text, length) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        close(fd[1]);
        if (wait(NULL) == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}
