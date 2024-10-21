#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int a = 0;

void func(int sig) {
    write(1, "\a", 1);
    signal(SIGINT, func);
    a++;
}

void func1(int sig) {
    char buf[20];
    int len;

    write(1, "\n", 1);

    len = snprintf(buf, sizeof(buf), "%d", a);

    write(1, buf, len);

    write(1, "\n", 1);
    exit(0);
}

int main() {
    signal(SIGINT, func);
    signal(SIGQUIT, func1);

    while (1) { pause(); };
    exit(0);
}

