#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

volatile sig_atomic_t counter = 0;

void sigHandler(int sig) {
    switch (sig) { 
    case SIGQUIT: ;
        char *message = "\nTotal beeps num = ";
        write(1, message, strlen(message));
        char num[4];
        int bytesToWrite = sprintf(num, "%d", counter);
        write(1, num, bytesToWrite);
        write(1, "\n", 1);

        exit(EXIT_SUCCESS) ;
    case SIGINT: ;
        write(1, "\a", 1);
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
}
