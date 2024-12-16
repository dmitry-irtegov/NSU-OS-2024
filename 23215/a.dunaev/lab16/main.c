#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

struct termios old, new;
char response;

// Function to restore terminal settings on exit
void restore_terminal() {
    if (tcsetattr(STDIN_FILENO, TCSANOW, &old)) {
        perror("Bad parameters or error in I/O");
        exit(1);        
    }
}

void handle(int sig) {
    // Set terminal to non-canonical mode
    //if (tcgetattr(STDIN_FILENO, &old)) {
    //    perror("Bad parameters");
    //    exit(1);
    //}
    new = old;
    new.c_lflag &= ~ICANON;
    new.c_cc[VMIN] = 1;
    new.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new)) {
        perror("Bad parameters or error in I/O");
        exit(1);
    }
    read(STDIN_FILENO, &response, 1);
}

int main() {
    if (tcgetattr(STDIN_FILENO, &old)) {
        perror("bad Param");
        exit(1);
    }
    printf("Enter a single character: ");
    fflush(stdout);

    // Set up signal handling
    signal(SIGCONT, handle);

    // Ensure terminal settings are restored on exit
    atexit(restore_terminal);

    // Initial terminal setup
    handle(SIGCONT);
   
    // Restore terminal settings before exiting
//    restore_terminal();

    printf("\nYou entered: %c\n", response);

    return 0;
}
