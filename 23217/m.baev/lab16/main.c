#include <stdio.h>
#include <termios.h>
#include <unistd.h>

void enableRawMode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void disableRawMode(struct termios *term) {
    tcsetattr(STDIN_FILENO, TCSANOW, term);
}

int main() {
    char response;
    struct termios orig_term;
    
    tcgetattr(STDIN_FILENO, &orig_term);

    enableRawMode();

    printf("Продолжить? (y/n): ");
    fflush(stdout);

    response = getchar();

    disableRawMode(&orig_term);

    printf("\nВы выбрали: %c\n", response);

    return 0;
}
