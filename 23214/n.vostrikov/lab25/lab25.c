#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define SIZE 7

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
        if (write(fd[1], text, len) == -1) {
            perror("write failed");
            exit(EXIT_FAILURE);
        }
        close(fd[1]);
        if(wait(NULL) == -1) {
            perror("wait failed");
            exit(EXIT_FAILURE);
        }
    }
    else if (pid == 0) {
        close(fd[1]);
        char upper_text[SIZE];
        while(read(fd[0], upper_text, SIZE) > 0) {
            for(int i = 0; SIZE > i && upper_text[i] != '\0'; ++i) {
                printf("%c", toupper(upper_text[i]));
            }
        }
        close(fd[0]);
    }
    exit(EXIT_SUCCESS);
}