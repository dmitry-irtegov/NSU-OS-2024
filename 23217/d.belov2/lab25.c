#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/wait.h>

#define BUFFER_SIZE 100

int main()
{
    int fildes[2];

    if (pipe(fildes) == -1)
    {
        perror("pipe error");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork error");
        return 1;
    }

    if (pid == 0)
    {
        close(fildes[1]); 

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;

        while ((bytes_read = read(fildes[0], buffer, BUFFER_SIZE)) > 0)
        {
            for (ssize_t i = 0; i < bytes_read; i++)
            {
                printf("%c", toupper(buffer[i])); 
            }
            printf("\n");
        }

        if (bytes_read == -1)
        {
            perror("read error");
        }

        close(fildes[0]); 
    }
    else 
    {
        close(fildes[0]); 

        char *text = "Hello, World!";
        size_t length = strlen(text); 

        size_t written = 0;
        while (written < length)
        {
            size_t chunk_size = (length - written > BUFFER_SIZE) ? BUFFER_SIZE : (length - written);
            if (write(fildes[1], text + written, chunk_size) == -1)
            {
                perror("write error");
                close(fildes[1]);
                return 1;
            }
            written += chunk_size;
        }

        close(fildes[1]); 
        wait(NULL); 
    }

    return 0;
}
