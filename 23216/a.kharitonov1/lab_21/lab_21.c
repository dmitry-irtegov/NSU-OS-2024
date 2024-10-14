#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int count;

void sigcatch(int sig)
{
    switch(sig){
        case SIGQUIT:
            char buf[40];
            if (sprintf(buf,"\n%d signals count\n", count) == -1){
                _exit(EXIT_FAILURE);
            }
            if (write(1, buf, strlen(buf)) == -1){
                _exit(EXIT_FAILURE);
            }
            _exit(EXIT_SUCCESS);
        case SIGINT:
            count++;
            write(1,"\a",1);
    }
}

void main()
{
    count = 0;
    sigset(SIGINT, sigcatch);
    sigset(SIGQUIT, sigcatch);
    while(1){
        pause();
    }
}