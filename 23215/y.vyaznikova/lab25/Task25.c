#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 10

int main() {
    int pipe_fd[2];

    if (pipe(pipe_fd) == -1) {
        perror("Failed to create pipe");
        exit(1);
    }

    pid_t subprocess_id = fork();
    
    if (subprocess_id == -1) {
        perror("Failed to create subprocess");
        exit(2);
    }

    if (subprocess_id == 0) {
        close(pipe_fd[1]);
        
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;

        while ((bytes_read = read(pipe_fd[0], buffer, BUFFER_SIZE)) > 0) {
            for (int i = 0; i < bytes_read; i++) {
                buffer[i] = toupper((unsigned char)buffer[i]);
            }
            fwrite(buffer, sizeof(char), bytes_read, stdout);
        }
        printf("\n");

        if (bytes_read == -1) {
            perror("Failed to read text from pipe");
            exit(3);
        }

        close(pipe_fd[0]);
    } else {
        close(pipe_fd[0]);

        const char *input_text = "TeXT iN DiFFeReNT ReGiSTeRS";
        size_t text_length = strlen(input_text);

        if (write(pipe_fd[1], input_text, text_length) == -1) {
            perror("Failed to write into pipe");
            exit(4);
        }

        close(pipe_fd[1]);
        if (wait(NULL) == -1) {
            perror("Failed to wait for child process");
            exit(5);
        }
    }

    exit(0);
}