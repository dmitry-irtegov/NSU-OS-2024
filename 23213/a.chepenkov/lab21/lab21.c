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
    char beepAmount[10];
    int index = 0;
    if (counter == 0) {
        beepAmount[index++] = '0';
    }
    while (counter > 0) {
        beepAmount[index++] = (counter % 10) + '0';
        counter /= 10;
    }
    index--;

    if (write(1, "\nAmount of beeps: ", 18) == -1) {
        _exit(EXIT_FAILURE);
    }

    for (int i = index; i >= 0; i--) {
        if (write(1, &beepAmount[i], 1) == -1) {
            _exit(EXIT_FAILURE);
        }
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
