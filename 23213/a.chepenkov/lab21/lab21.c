#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int counter = 0;

void handle_signal(int sig) {
    if (sig == SIGQUIT) {
        char output[11];
        int len = sprintf(output, "%d", counter);

        write(1, "\nAmount of beeps: ", 18);

        write(1, output, len);

        write(1, "\n", 1);

        _exit(EXIT_SUCCESS);
    }
    if (sig == SIGINT) {
        write(1, "\a", 1);
        counter++;
    }
}

int main() {
    if (sigset(SIGINT, handle_signal) == SIG_ERR) {
        perror("Failed to set SIGINT handler");
        exit(EXIT_FAILURE);
    }

    if (sigset(SIGQUIT, handle_signal) == SIG_ERR) {
        perror("Failed to set SIGQUIT handler");
        exit(EXIT_FAILURE);
    }

    while (1) {
        pause();
    }
}
