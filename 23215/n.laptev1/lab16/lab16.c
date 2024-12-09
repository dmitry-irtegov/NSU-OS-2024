#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

struct termios original_terminal_attributes;

void sigcont_handler() {
    tcsetattr(0, TCSANOW, &original_terminal_attributes);
}

int main() {
    struct termios terminal_attributes;
    if (tcgetattr(0, &terminal_attributes) == -1) {
        perror("tcgetattr failed");
        exit(EXIT_FAILURE);
    }

    original_terminal_attributes = terminal_attributes;
    original_terminal_attributes.c_lflag &= ~(ICANON);
    original_terminal_attributes.c_cc[VMIN] = 1;

    if (tcsetattr(0, TCSANOW, &original_terminal_attributes) == -1) {
        perror("tcsetattr failed.");
        exit(EXIT_FAILURE);
    }

    signal(SIGCONT, sigcont_handler);

    printf("How many team trophies does Harry Kane have?\n");

    char amount_of_cups = getchar();

    switch (amount_of_cups) {
        case '0':
            printf("\nCorrect, he doesn't have any team cups.\n");
            break;
        default:
            printf("\nThat's not correct. He has 0 cups.\n");
            break;
    }

    if (tcsetattr(0, TCSANOW, &terminal_attributes) == -1) {
        perror("tcsetattr failed.");
        exit(EXIT_FAILURE);
    }

    return 0;
}
