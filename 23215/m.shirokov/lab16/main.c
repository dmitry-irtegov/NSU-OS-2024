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
    char answer;
    set_terminal_mode(&old_tio);
    printf("Yes or no? (y/n): ");
    answer = getchar();
    printf("\n");
    restore_terminal_mode(&old_tio);
    if (answer == 'y' || answer == 'Y') {
        printf("Yes!\n");
    } else if (answer == 'n' || answer == 'N') {
        printf("No!\n");
    } else {
        printf("Invalid response: %c\n", answer);
    }
    return 0;
}