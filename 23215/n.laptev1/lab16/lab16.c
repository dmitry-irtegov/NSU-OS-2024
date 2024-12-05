#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

int main() {
    struct termios terminal_attributes, new_terminal_attributes;
    
    if (tcgetattr(0, &terminal_attributes) == -1) {
        perror("tcgetattr failed");
        exit(EXIT_FAILURE);
    }

    new_terminal_attributes = terminal_attributes;
    new_terminal_attributes.c_lflag &= ~(ICANON);
    new_terminal_attributes.c_cc[VMIN] = 1;
    
    if (tcsetattr(0, TCSANOW, &new_terminal_attributes) == -1) {
        perror("tcsetattr failed.");
        exit(EXIT_FAILURE);
    }

    printf("How many team trophies does Harry Kane have? ");
    char amount_of_cups = getchar();

    
    if (tcsetattr(0, TCSANOW, &terminal_attributes) == -1) {
        perror("tcsetattr failed.");
        exit(EXIT_FAILURE);
    }

    switch(amount_of_cups) {
        case '0':
            printf("\nCorrect, he doesn't have any team cups.\n");
            break;
        default:
            printf("\nThat's not correct. He has 0 cups.\n");
            break;
    } 

    return 0;
}
