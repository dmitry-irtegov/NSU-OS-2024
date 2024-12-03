#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int counter = 0;

void sigHandler(int sig) {
    switch (sig) { 
    case SIGQUIT: ;
        printf("Total num of beeps: %d", counter);
        return ;
    case SIGINT: ;
        printf("\a");
        counter++;
    }
}

int main() {
    if (sigset(SIGINT, sigHandler) == SIG_ERR) {
        perror("Failed to set SIGINT handler");
        exit(EXIT_FAILURE);
    }

    if (sigset(SIGQUIT, sigHandler) == SIG_ERR) {
        perror("Failed to set SIGQUIT handler");
        exit(EXIT_FAILURE);
    }

    while (1) {
        pause();
    }

    exit(EXIT_SUCCESS);
}
