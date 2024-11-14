#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

int main(){

    struct termios newTerm;
    struct termios oldTerm;

    if(isatty(fileno(stdin)) == 0){
        fprintf(stderr, "input not refers to a terminal\n");
        exit(1);
    }

    tcgetattr(fileno(stdin), &oldTerm);

    newTerm = oldTerm;
    newTerm.c_lflag &= ~(ICANON | ISIG);
    newTerm.c_cc[VMIN] = 1;

    tcsetattr(fileno(stdin), TCSANOW, &newTerm);

    printf("?\n");
    char in = getchar();
    printf("%c\n", in);
    tcsetattr(fileno(stdin), TCSANOW, &oldTerm);

    exit(0);
}