#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>

int main() {
    struct termios new_tty, old_tty;
    int c = fileno(stdin);

    if (isatty(c) == 0) {
        fprintf(stderr, "stdin not terminal\n");
        exit(EXIT_FAILURE);
    }

    if (tcgetattr(c, &old_tty) == -1) {
        perror("tcgetattr fail");
        exit(EXIT_FAILURE);
    }

    new_tty = old_tty;
    new_tty.c_lflag &= ~(ISIG | ICANON);
    new_tty.c_cc[VMIN] = 1;

    if (tcsetattr(c, TCSANOW, &new_tty) == -1) {
        perror("tcsetattr fail");
        exit(EXIT_FAILURE);
    }

    printf("Hello!!!\n");
    char input = getchar();
    printf("\nwhat??\n");

    if (tcsetattr(c, TCSANOW, &old_tty) == -1) {
        perror("tcsetattr fail");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
