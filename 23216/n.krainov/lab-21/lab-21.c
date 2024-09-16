#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

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
    if (sigset(SIGQUIT, sigquitHandler) == SIG_ERR){
        perror("sigset failed");
        exit(EXIT_FAILURE);
    }
    if (sigset(SIGINT, sigintHandler) == SIG_ERR){
        perror("sigset failed");
        exit(EXIT_FAILURE);
    }

    while (1){
        pause();
    }
}