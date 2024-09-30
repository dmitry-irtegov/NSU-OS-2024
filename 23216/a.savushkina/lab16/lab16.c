#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>

int main() {
    struct termios tty, savtty;

    char line[30] = "Print any symbol\n";

    fputs(line, stdout);

    if (tcgetattr(fileno(stdin), &tty) == -1) {
        perror("Terminal get attr error.");
        exit(EXIT_FAILURE);
    }

    savtty = tty;

    tty.c_lflag &= ~(ISIG | ICANON | ECHO);
    tty.c_cc[VMIN] = 1;

    if (tcsetattr(fileno(stdin), TCSANOW, &tty) == -1) {
        perror("Terminal set attr error.");
        exit(EXIT_FAILURE);
    }
    char char_tty = fgetc(stdin);

    if (char_tty) {
        sprintf(line, "Your symbol is %c\n", char_tty);
        fputs(line, stdout);
        tty.c_cc[VMIN] = 0;
        if (tcsetattr(fileno(stdin), TCSANOW, &tty) == -1) {
            perror("Terminal set attr error.");
            exit(EXIT_FAILURE);
        }
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