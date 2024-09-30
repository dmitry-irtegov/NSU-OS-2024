#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int counter = 0;

void handleSigint() {
    write(1, "\a", 1);
    counter++;
}

void handleSigquit() {
    if (write(1, "\nAmount of beeps: ", 18) == -1) {
        _exit(EXIT_FAILURE);
    }

    if (write(1, &counter, 4) == -1) {
        _exit(EXIT_FAILURE);
    }

    if (write(1, "\n", 1) == -1) {
        _exit(EXIT_FAILURE);
    }
    _exit(EXIT_SUCCESS);
}

int main() {
    struct sigaction sigint_action, sigquit_action;

    sigint_action.sa_handler = handleSigint;
    sigquit_action.sa_handler = handleSigquit;

    if (sigaction(SIGINT, &sigint_action, NULL) == -1) {
        perror("Failed to set SIGINT handler");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGQUIT, &sigquit_action, NULL) == -1) {
        perror("Failed to set SIGQUIT handler");
        exit(EXIT_FAILURE);
    }

    while (1) {
        pause();
    }
}
