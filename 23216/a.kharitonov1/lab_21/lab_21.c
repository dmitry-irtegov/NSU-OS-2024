#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int count;

void sigcatch(int sig)
{
    char buf[40];
    write(1,"\n",1);
    sleep(3);
    switch(sig){
        case SIGQUIT:
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

int main()
{
    struct sigaction signalint;
    struct sigaction signalquit;
    memset(&signalint, 0, sizeof(signalint));
    memset(&signalint, 0, sizeof(signalquit));
    sigset_t masksignalint;
    sigset_t masksignalquit;
    sigemptyset(&masksignalint);
    sigemptyset(&masksignalquit);
    sigaddset(&masksignalquit, SIGQUIT);
    signalint.sa_handler = sigcatch;
    signalquit.sa_handler = sigcatch;
    signalint.sa_mask = masksignalint;
    signalquit.sa_mask = masksignalquit;
    signalint.sa_flags = SA_NODEFER;
    count = 0;
    if (sigaction(SIGINT, &signalint, NULL)){
        perror("sigaction failed");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGQUIT, &signalquit, NULL)){
        perror("sigaction failed");
        exit(EXIT_FAILURE);
    }
    while(1){
        pause();
    }
}