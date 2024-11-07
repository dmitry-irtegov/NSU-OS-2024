#include <stdio.h>
#include <termios.h>
#include <unistd.h>

int main() {
    struct termios oldt, newt;
    char answer;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = 1; 
    newt.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    printf("Enter symbol: ");
    fflush(stdout);

    answer = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    printf("\nYou have entered a symbol: %c\n", answer);

    return 0;
}
