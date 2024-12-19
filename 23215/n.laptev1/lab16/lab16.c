#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

struct termios original_terminal_attributes;

void sigcont_handler(int signum) {
        if(tcsetattr(0, TCSANOW, &original_terminal_attributes) == -1) {
           perror("tcsetattr failed in sigcont_handler");
           exit(EXIT_FAILURE);
     }
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

    struct sigaction sa;
    sa.sa_handler = sigcont_handler;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCONT, &sa, NULL);
    
    if (tcsetattr(0, TCSANOW, &original_terminal_attributes) == -1) {
        perror("tcsetattr for new terminal attributes failed.");
        exit(EXIT_FAILURE);
    }

    printf("How many team trophies does Harry Kane have?\n");
    char amountOfCups = fgetc(stdin);
    switch (amountOfCups) {
        case '0':
            printf("\nCorrect, he doesn't have any team cups.\n");
            break;
        default:
            printf("\nThat's not correct. He has 0 cups.\n");
            break;
    }

    if (tcsetattr(0, TCSANOW, &terminal_attributes) == -1) {
        perror("tcsetattr for restoring terminal attributes failed.");
        exit(EXIT_FAILURE);
    }

    return 0;
}
