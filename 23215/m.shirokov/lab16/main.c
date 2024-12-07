#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

void set_terminal_mode(struct termios *old_tio) {
    struct termios new_tio;

    if (tcgetattr(STDIN_FILENO, old_tio) != 0) {
        perror("tcgetattr failed");
        exit(EXIT_FAILURE);
    }
    new_tio = *old_tio;
    new_tio.c_lflag &= ~(ICANON);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_tio) != 0) {
        perror("tcsetattr failed");
        exit(EXIT_FAILURE);
    }
}

void restore_terminal_mode(struct termios *old_tio) {
    if (tcsetattr(STDIN_FILENO, TCSANOW, old_tio) != 0) {
        perror("tcsetattr restore failed");
        exit(EXIT_FAILURE);
    }
}
int main() {
    struct termios old_tio;
    set_terminal_mode(&old_tio);
    printf("Yes or no? (y/n): ");
    static char answer[2];
    fgets(answer,2,stdin);
    restore_terminal_mode(&old_tio);
    printf("\n");
    if (answer[0] == 'y' || answer[0] == 'Y') {
        printf("Yes!\n");
    } else if (answer[0] == 'n' || answer[0] == 'N') {
        printf("No!\n");
    } else {
        printf("Invalid response: %c\n", answer[0]);
    }
    return 0;
}