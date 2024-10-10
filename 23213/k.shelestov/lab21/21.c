#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

int sigint_count = 0;

void handle_sigint(int sig) {
    sigint_count++;
    write(STDOUT_FILENO, "\a", 1);
}

void handle_sigquit(int sig) {
    char buffer[256];
    int n = snprintf(buffer, sizeof(buffer), "\n SIGQUIT received!\n SIGINT received %d times.\n", sigint_count);
    write(STDOUT_FILENO, buffer, n);
    _exit(EXIT_SUCCESS);
}

int main() {
    if (sigset(SIGINT, handle_sigint) == SIG_ERR) {
        perror("signal error for SIGINT");
        exit(EXIT_FAILURE);
    }

    if (sigset(SIGQUIT, handle_sigquit) == SIG_ERR) {
        perror("signal error for SIGQUIT");
        exit(EXIT_FAILURE);
    }

    while (1) { 
        pause(); 
    }
}
