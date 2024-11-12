#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <stdlib.h>

int main() {
    struct termios termios, normal_termios;
    tcgetattr(0, &termios);
    normal_termios = termios;
    termios.c_lflag &= ~ICANON;
    termios.c_iflag |= IUCLC;
    tcsetattr(0, TCSANOW, &termios);
    char answer;
    do {
        printf("Do you want to exit the programm? (y/n) ");
        answer = getc(stdin);
        printf("\n");
    } while (answer != 'y');
    tcsetattr(0, TCSANOW, &normal_termios);
    exit(EXIT_SUCCESS);
}
