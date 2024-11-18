#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

int main() {
    char* text = "uGaBuGa";
    size_t len = strlen(text);
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) {
        close(fd[0]);
        if (write(fd[1], text, len + 1) == -1) {
            perror("write failed");
            exit(EXIT_FAILURE);
        }
        close(fd[1]);
    }
    else if (pid == 0) {
        close(fd[1]);
        char upper_text[len+1];
        if (read(fd[0], upper_text, len + 1) == -1) {
            perror("read failed");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < len; ++i) {
            upper_text[i] = toupper(upper_text[i]);
        }
        printf("%s\n", upper_text);
        close(fd[0]);
    }
    exit(EXIT_SUCCESS);
}