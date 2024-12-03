#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>

#define MSG_SIZE 21

int main()
{
    int pipe_fd[2];
    pid_t pid;
    char buffer[MSG_SIZE];
    const char *text = "Message to converted.\n";

    if (pipe(pipe_fd) == -1)
    {
        perror("ERROR with pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid < 0)
    {
        perror("ERROR with fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        close(pipe_fd[1]);

        ssize_t bytes_read;
        while ((bytes_read = read(pipe_fd[0], buffer, MSG_SIZE)) > 0)
        {
            for (ssize_t i = 0; i < bytes_read; ++i)
            {
                buffer[i] = toupper((unsigned char)buffer[i]);
            }
            if (write(STDOUT_FILENO, buffer, bytes_read) == -1)
            {
                perror("ERROR with write");
                exit(EXIT_FAILURE);
            }
        }

        if (bytes_read == -1)
        {
            perror("ERROR with read");
            exit(EXIT_FAILURE);
        }

        close(pipe_fd[0]);
        exit(EXIT_SUCCESS);
    }
    else
    {
        close(pipe_fd[0]);

        size_t text_len = strlen(text);
        if (write(pipe_fd[1], text, text_len) == -1)
        {
            perror("ERROR with write");
            exit(EXIT_FAILURE);
        }

        close(pipe_fd[1]);

        if (wait(NULL) == -1)
        {
            perror("ERROR with wait");
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}