#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int count = 0;
int flag = 1;

void handle_sigquit() {
    flag = 0;
}

void handle_sigint() {
    count++;
    write(1, "\a", 1);
}

int main() {
    struct sigaction sa_quit, sa_int;
    sa_quit.sa_handler = handle_sigquit;
    sa_int.sa_handler = handle_sigint;
    sigaction(SIGQUIT, &sa_quit, NULL);
    sigaction(SIGINT, &sa_int, NULL);

    while (1) {
        if (flag == 0) {
            printf("\nThe beep sounded %d times\n", count);
            exit(0);
        }
    }

    return 0;
}