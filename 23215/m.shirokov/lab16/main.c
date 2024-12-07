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
    new_tio.c_cc[VMIN] = 1;
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
    char answer;
    set_terminal_mode(&old_tio);
    printf("Yes or no? (y/n): ");
    fflush(stdout);
    if (read(STDIN_FILENO, &answer, 1) != 1) {
        perror("read failed");
        restore_terminal_mode(&old_tio);
        exit(EXIT_FAILURE);
    }

    restore_terminal_mode(&old_tio);
    printf("\n");
    if (answer == 'y' || answer == 'Y') {
        printf("Yes!\n");
    } else if (answer == 'n' || answer == 'N') {
        printf("No!\n");
    } else {
        printf("Invalid response: %c\n", answer);
    }

    return 0;
}
