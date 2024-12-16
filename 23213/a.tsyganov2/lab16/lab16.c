#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int fd;
    struct termios tty, savtty;
    char ch;

    fd = fileno(stdin);
    if (isatty(fd) == 0) {
        fprintf(stderr, "stdin not terminal\n");
        exit(1);
    }

    if (tcgetattr(fd, &tty) != 0) {
        perror("can't get attributes\n");
        exit(1);
    }

    savtty = tty;
    tty.c_lflag &= ~(ICANON|ISIG);
    tty.c_cc[VMIN] = 1;
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("can't set attributes\n");
        exit(1);
    }

    setbuf(stdout, (char*)NULL);
    printf("What's the third letter in the word TERMINAL?\n");
    read(fd, &ch, 1);
    printf("\n");
    if (ch == 'R') {
        printf("Correct!\n");
    } else {
        printf("Wrong.\n");
    }

    if(tcsetattr(fd, TCSANOW, &savtty) != 0) {
        perror("can't set old attributes\n");
        exit(1);
    }
}
