#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#define STRING_SIZE 12

int main() {
    char test_string[STRING_SIZE] = "test_string";

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        printf("Failed to create pipe.\n");
        exit(-1);
    }

    pid_t pid = fork();

    if (!pid) {
        close(pipe_fd[0]);

        char read_string[STRING_SIZE];
        ssize_t read_result = read(pipe_fd[1], read_string, STRING_SIZE);
	
        if (read_result == -1) {
            printf("Error reading file.\n");
            exit(-1);
        }

        for (int i = 0; i < read_result; i++) {
            printf("%c", toupper(read_string[i]));
        }

        printf("\n");
        close(pipe_fd[1]);
    }

    else if (pid > 0) {
        close(pipe_fd[1]);

        if (write(pipe_fd[0], test_string, STRING_SIZE) == -1) {
            printf("Error writing in pipe.\n");
            exit(-1);
        }

        close(pipe_fd[0]);
    }

    else {
        printf("Error creating subprocess.\n");
        exit(-1);
    }

    exit(0);
}
