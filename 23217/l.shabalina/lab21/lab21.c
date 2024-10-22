#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

int beep_count = 0;

void handle_signal(int sig) {
    if (sig == SIGQUIT) {
        char buffer[50];
        int len = snprintf(buffer, sizeof(buffer), "\nAmount of beeps: %d\n", beep_count);
        write(STDOUT_FILENO, buffer, len);
        exit(EXIT_SUCCESS);
    }
    else if (sig == SIGINT) {
        write(STDOUT_FILENO, "\007", 1);
        beep_count++;
        signal(SIGINT, handle_signal);
    }
}

void main() {

    if (signal(SIGINT, handle_signal) == SIG_ERR) {
        perror("Fail to set SIGINT handler");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGQUIT, handle_signal) == SIG_ERR) {
        perror("Fail to set SIGQUIT handler");
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        pause();
    }

}
