#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>

int main() {
    struct termios tty, savtty;
    printf("Print any symbol\n");

    if (tcgetattr(fileno(stdin), &tty) == -1) {
        perror("Terminal get attr error.");
        exit(EXIT_FAILURE);
    }

    savtty = tty;

    tty.c_lflag &= ~(ECHO);
    tty.c_cc[VMIN] = 1;

    if (tcsetattr(fileno(stdin), TCSANOW, &tty) == -1) {
        perror("Terminal set attr error.");
        exit(EXIT_FAILURE);
    }
    char char_tty = fgetc(stdin);

    if (char_tty) {
        printf("Your symbol is %c\n", char_tty);
    } else {
        perror("fgets error.");
        exit(EXIT_FAILURE);
    }

    if (tcsetattr(fileno(stdin), TCSANOW, &savtty) == -1) {
        perror("Terminal set attr error.");
        exit(EXIT_FAILURE);
    }

    return 0;
}