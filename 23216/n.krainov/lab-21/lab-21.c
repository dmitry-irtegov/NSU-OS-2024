#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

int counter = 0;

void sigintHandler(){
    write(1, "\a", 1);
    counter++;
}

void sigquitHandler(){
    char num[20];
    int length = 0;
    while (counter > 0){
        num[length] = counter%10 + '0';
        counter/=10;
        length++;
    }

    if (write(1, "\ncounts of interrupts:", 22) == -1){
        _exit(EXIT_FAILURE);
    }

    for (int i = length - 1; i>=0; i--){
        if (write(1, &num[i], 1) == -1){
            _exit(EXIT_FAILURE);
        }
    }

    if (write(1, "\n", 1) == -1){
        _exit(EXIT_FAILURE);
    }

    _exit(EXIT_SUCCESS);
}


int main(){
    struct sigaction signalint;
    struct sigaction signalquit;

    memset(&signalint, 0, sizeof(signalint));
    memset(&signalquit, 0, sizeof(signalquit));

    sigset_t masksignalint;
    sigset_t masksignalquit;

    sigemptyset(&masksignalint);
    sigemptyset(&masksignalquit);

    sigaddset(&masksignalint, SIGQUIT);
    sigaddset(&masksignalquit, SIGQUIT);
    sigaddset(&masksignalquit, SIGINT);

    signalint.sa_handler = sigintHandler;
    signalquit.sa_handler = sigquitHandler;
    signalint.sa_mask = masksignalint;
    signalquit.sa_mask = masksignalquit;

    if (sigaction(SIGINT, &signalint, NULL)){
        perror("sigaction failed");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGQUIT, &signalquit, NULL)){
        perror("sigaction failed");
        exit(EXIT_FAILURE);
    }

    while (1){
        pause();
    }
}