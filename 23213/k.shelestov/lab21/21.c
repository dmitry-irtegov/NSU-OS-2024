#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

volatile sig_atomic_t sigint_count = 0;
volatile sig_atomic_t running = 1;  

void handle_sigint(int sig) {
    sigint_count++;
    if (write(STDOUT_FILENO, "\a", 1) != 1) {
        perror("write error in handle_sigint");
    }
}

void handle_sigquit(int sig) {
    char buffer[256];
    int n = snprintf(buffer, sizeof(buffer), "\nСигнал SIGQUIT получен!\nСигнал SIGINT получен %d раз.\n", sigint_count);
    if (write(STDOUT_FILENO, buffer, n) != n) {
        perror("write error in handle_sigquit");
    }
    running = 0; 
}

int main() {
    struct sigaction sa_sigint;
    sa_sigint.sa_handler = handle_sigint;
    sigemptyset(&sa_sigint.sa_mask);
    sa_sigint.sa_flags = 0;

    if (sigaction(SIGINT, &sa_sigint, NULL) == -1) {
        perror("sigaction error for SIGINT");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa_sigquit;
    sa_sigquit.sa_handler = handle_sigquit;
    sigemptyset(&sa_sigquit.sa_mask);
    sa_sigquit.sa_flags = 0;

    if (sigaction(SIGQUIT, &sa_sigquit, NULL) == -1) {
        perror("sigaction error for SIGQUIT");
        exit(EXIT_FAILURE);
    }

    while (running) { 
        pause(); 
    }

    exit(0);  
}
