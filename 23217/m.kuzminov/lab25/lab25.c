#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

int main() {
    char* string = "HeHe hAhA";
    int size = strlen(string) + 1;
    int fd[2];

    if(pipe(fd) == -1) {
        fprintf(stderr, "Can't create a pipe\n");
        exit(1);
    }

    pid_t pid = fork();

    if(pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid > 0) { 
        close(fd[0]);
        if (write(fd[1], string, size) == -1) {
            perror("write failed");
            exit(1);
        }
        close(fd[1]);
    } else { 
        close(fd[1]);
        char getString[size];
        if (read(fd[0], getString, size) == -1) {
            perror("read failed");
            exit(1);
        }
        
        for(int i = 0; i < size - 1; i++) {
            printf("%c", toupper(getString[i]));
        }
        printf("\n");
        close(fd[0]); 
    }

    return 0;
}
