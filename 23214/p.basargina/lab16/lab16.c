#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    struct termios oldt, newt;
    char ch;
    int fd;
    fd = fileno(stdin);

    if (isatty(fd) == 0) {
        perror("Standart input is not terminal");
        exit(1);
    }

    if (tcgetattr(fd, &oldt) != 0) {
        perror("Error getting terminal attributes");
        exit(2);
    }
    
    newt = oldt;

    newt.c_lflag &= ~(ICANON);
    newt.c_cc[VMIN] = 1;
    
    if (tcsetattr(fd, TCSANOW, &newt) != 0) {
        perror("Error while setting terminal attributes");
        exit(3);
    }

    printf("Enter something: ");

    ch = getchar();
    
    if (ch == EOF) {
        perror("Error reading symbol");
        tcsetattr(fd, TCSANOW, &oldt);
        exit(4);
    }

    if (tcsetattr(fd, TCSANOW, &oldt) != 0) {
        perror("Error while restoring terminal attributes");
        exit(5);
    }

    printf("\nYou entered: %c\n", ch);

    exit(0);
}
