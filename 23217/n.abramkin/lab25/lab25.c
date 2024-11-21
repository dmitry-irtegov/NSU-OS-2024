#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int pipe_fd[2];  // Array to hold read and write file descriptors for the pipe
    pid_t pid;

    // Create the pipe
    if (pipe(pipe_fd) == -1) {
        perror("Failed to create pipe");
        return 1;
    }

    // Create a child process
    pid = fork();
    if (pid < 0) {
        perror("Failed to fork");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return 1;
    } else if (pid == 0) {
        // Child process: reads from the pipe and converts text to uppercase
        close(pipe_fd[1]);  // Close the write end

        char buffer[256];
        int bytesRead;

        while ((bytesRead = read(pipe_fd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            // Convert each character to uppercase
            for (int i = 0; i < bytesRead; i++) {
                buffer[i] = toupper(buffer[i]);
            }
            printf("Child process received text: %s\n", buffer);
        }

        if (bytesRead == -1) {
            perror("Error reading from pipe");
        }

        close(pipe_fd[0]);  // Close the read end
        return 0;           
    } else {
        // Parent process: writes to the pipe
        close(pipe_fd[0]);  // Close the read end

        const char *text = "Sample text with Mixed Case.\n";
        size_t textLen = strlen(text);
        ssize_t bytesWritten = 0;

        // Write in a loop to ensure all bytes are written
        while (bytesWritten < (ssize_t)textLen) {
            ssize_t result = write(pipe_fd[1], text + bytesWritten, textLen - bytesWritten);
            if (result == -1) {
                perror("Error writing to pipe");
                close(pipe_fd[1]);  // Close the write end
                return 1;
            }
            bytesWritten += result;
        }

        close(pipe_fd[1]);  // Close the write end

        // Wait for the child process to complete
        if (wait(NULL) == -1) {
            perror("Error waiting for child process");
        }
    }

    return 0;
}
