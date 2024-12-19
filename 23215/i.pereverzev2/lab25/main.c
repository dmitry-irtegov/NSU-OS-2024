#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXLENGTH 8

int main()
{
    int fildes[2];
    if (pipe(fildes) == -1) {
        perror("unable to create pipe");
        return 1;
    }
    pid_t chpid = fork();
    if (chpid == -1) {
        perror("unable to create child process");
        return 2;
    }
    if (chpid == 0) {
        close(fildes[1]);
        char buf[] = "Sample Text That Will Be Written inTo the pipe";
        if (write(fildes[0], buf, strlen(buf)) == -1) {
            perror("unable to write into the pipe from child");
            return 3;
        }
        close(fildes[0]);
    } else {
        close(fildes[0]);
        char buf[MAXLENGTH];
        ssize_t readed = 0;
        while((readed = read(fildes[1], buf, MAXLENGTH - 1)) > 0) {
            for(int i = 0; i < readed; i++) {
                buf[i] = toupper(buf[i]);
                printf("%c", buf[i]);
            }
        }
        printf("\n");
        if(readed == -1) {
            perror("unable to read");
            return 4;
        }
        close(fildes[1]);
    }
    return 0;
}