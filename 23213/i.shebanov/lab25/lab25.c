#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>


#define BUFF_SIZE 1024
int main()
{
    pid_t pid;
    int fd[2];

    if(pipe(fd) == -1)
    {
        perror("pipe creation error");
        exit(EXIT_FAILURE);
    }

    pid = fork();

    if(pid == -1)
    {
        perror("fork error");
        exit(EXIT_FAILURE);
    }

    if(pid == 0)
    {
        close(fd[0]);

        char buff[BUFF_SIZE];
        ssize_t bytes_read;
        while((bytes_read = read(STDIN_FILENO, buff, BUFF_SIZE)) > 0)
        {
            if (write(fd[1], buff, bytes_read) == -1) 
            {
                perror("write error");
                exit(EXIT_FAILURE);
            }
        }

        if(bytes_read == -1)
        {
            perror("read error");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

    close(fd[1]);

    char buff[BUFF_SIZE];
    ssize_t bytes_read;
    while((bytes_read = read(fd[0], buff, BUFF_SIZE)) > 0)
    {
        for (int i = 0; i < bytes_read; i++)
        {
            putchar(toupper((unsigned char)buff[i]));
        }
    }

    if(bytes_read == -1)
    {
        perror("read error");
        exit(EXIT_FAILURE);
    }

    if (wait(NULL) == -1)
    {
        perror("Child process hasn't finished successfuly");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
