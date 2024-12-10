#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

int count;
char buffer[BUFSIZ];

void sig_catch_int() {
    write(1, "\a", 1);
    count++;
}

void sig_catch_quit() {
    sprintf(buffer, "\nthe bell was called: %d times\n", count);
    write(1, buffer, strlen(buffer)+1);
    _exit(0);
}

int main() {
    count = 0;
    if ((sigset(SIGINT, sig_catch_int)) == SIG_ERR) {
        perror("sigset int error");
        _exit(1);
    }
    if ((sigset(SIGQUIT, sig_catch_quit)) == SIG_ERR) {
        perror("sigset quit error");
        _exit(2);
    }
    while (1) {
        pause();
    }
}

