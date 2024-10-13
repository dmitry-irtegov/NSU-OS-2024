#include <stdio.h>
#include <signal.h>
int count;

void sigcatch(int sig)
{
    char buf[40];
    signal(sig, SIG_IGN);
    switch(sig){
        case SIGQUIT:
            if (sprintf(buf,"\n%d signals count\n", count) == -1){
                _exit(EXIT_FAILURE);
            }
            if (write(1, buf, strlen(buf)) ==1){
                _exit(EXIT_FAILURE);
            }
            _exit(EXIT_SUCCESS);
        case SIGINT:
            count++;
            write(1,"\a",1);
    }
    signal(sig, sigcatch);
}

void main()
{
    count = 0;
    signal(SIGINT, sigcatch);
    signal(SIGQUIT, sigcatch);
    while(1){
        pause();
    }
}