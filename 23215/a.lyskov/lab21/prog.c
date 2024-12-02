#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

volatile sig_atomic_t sigint_count = 0;

void handle_sigint(int sig) {
    sigint_count++;
    const char beep_msg[] = "\a";
    write(STDOUT_FILENO, beep_msg, sizeof(beep_msg) - 1);
}

void handle_sigquit(int sig) {
    char buffer[100];
    int len = snprintf(buffer, sizeof(buffer), "\nNumber of SIGINT signals: %d\n", sigint_count);
    write(STDOUT_FILENO, buffer, len);
    exit(0);
}

int main() {
    struct sigaction sa_int, sa_quit;

    sa_int.sa_handler = handle_sigint;
    sa_int.sa_flags = 0;
    sigemptyset(&sa_int.sa_mask);
    sigaction(SIGINT, &sa_int, NULL);

    sa_quit.sa_handler = handle_sigquit;
    sa_quit.sa_flags = 0;
    sigemptyset(&sa_quit.sa_mask);
    sigaction(SIGQUIT, &sa_quit, NULL);

    const char start_msg[] = "The program is running. Press CTRL-C for a beep and CTRL-\\ to quit.\n";
    write(STDOUT_FILENO, start_msg, sizeof(start_msg) - 1);

    while (1) {
        pause();
    }

    return 0;
}
