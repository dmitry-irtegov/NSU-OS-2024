#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(){
    struct termios oldTerminal, newTerminal;

    if (!isatty(STDIN_FILENO)){
        fprintf(stderr, "Input is not a terminal\n");
        exit(EXIT_FAILURE);
    }

    if (tcgetattr(STDIN_FILENO, &oldTerminal) != 0){
        perror("Error getting terminal settings");
        exit(EXIT_FAILURE);
    }

    newTerminal = oldTerminal;
    newTerminal.c_lflag &= ~(ICANON | ECHO);
    newTerminal.c_cc[VMIN] = 1;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &newTerminal) != 0){
        perror("Error changing terminal settings");
        exit(EXIT_FAILURE);
    }

    printf("?\n");
    char input = getchar();
    printf("\nYou pressed: %c\n", input);

    if (tcsetattr(STDIN_FILENO, TCSANOW, &oldTerminal) != 0){
        perror("Error restoring terminal settings");
        exit(EXIT_FAILURE);
    }

    return 0;
}