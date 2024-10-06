#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
    int stdin_fd = 0;

    if (isatty(stdin_fd) == 0){
        perror("input isn't terminal");
        exit(EXIT_FAILURE);
    }

    struct termios attr, old;

    if (tcgetattr(stdin_fd, &attr)){
        perror("tcgetattr failed");
        exit(EXIT_FAILURE);
    }

    old = attr;
    attr.c_lflag &= ~(ICANON | ISIG);
    attr.c_cc[VMIN] = 1;


    if (tcsetattr(stdin_fd, TCSANOW, &attr)) {
        perror("failed tcsetattr");
        exit(EXIT_FAILURE);
    }

    puts("Press a button");
    char symbol;
    if (read(stdin_fd, &symbol, 1) == -1) {
        perror("failed read");
        if (tcsetattr(STDIN_FILENO, TCSANOW, &old) == -1) {
            perror("failed tcsetattr");
        }
        exit(EXIT_FAILURE);
    }

    puts("\nThank you!");

    if (tcsetattr(STDIN_FILENO, TCSANOW, &old) == -1) {
        perror("failed tcsetattr");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}