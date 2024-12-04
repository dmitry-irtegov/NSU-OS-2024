#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

int count = 0;

void handle_sigquit(int sig) {
    char buffer[50];
    int len = snprintf(buffer, sizeof(buffer), "\nСколько раз пропищало: %d\n", count);
    write(STDOUT_FILENO, buffer, len);
    exit(EXIT_SUCCESS);
}

void handle_sigint(int sig) {
    write(STDOUT_FILENO, "\007", 1);
    sleep(5);
    count++;
    signal(SIGINT, handle_sigint);
}

void main() {

    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        perror("Fail to set SIGINT handler");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGQUIT, handle_sigquit) == SIG_ERR) {
        perror("Fail to set SIGQUIT handler");
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        pause();
    }

}