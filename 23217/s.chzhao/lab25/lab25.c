#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

void to_upper_and_output(const char *input, ssize_t length) {
    for (ssize_t i = 0; i < length; i++) {
        putchar(toupper((unsigned char)input[i]));
    }
}

int main() {
    int pipe_fd[2];
    pid_t child_pid;
    const char *message = "Hello, Pipe! Mixed CASE text here.\n";
    size_t message_len = strlen(message);

    // 创建管道
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // 创建子进程
    child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0) {
        close(pipe_fd[1]);
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(pipe_fd[0], buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            to_upper_and_output(buffer, bytes_read);
        }
        close(pipe_fd[0]);
        exit(EXIT_SUCCESS);
    }

    // 父进程：关闭读端，向管道中写入数据
    close(pipe_fd[0]);
    // write(pipe_fd[1], message, message_len);
    size_t total_written = 0;
    while (total_written < message_len) {
        ssize_t written = write(pipe_fd[1], message + total_written, message_len - total_written);
        if (written == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        total_written += written;
    }
    close(pipe_fd[1]);
    wait(NULL);

    return 0;
}
