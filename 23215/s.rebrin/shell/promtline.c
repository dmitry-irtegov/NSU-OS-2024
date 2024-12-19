#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include "shell.h"

int promptline(char* prompt, char* line, int sizline) {
    struct termios orig_termios, new_termios;
    int n = 0;

    // Save original terminal attributes and configure raw mode
    tcgetattr(STDIN_FILENO, &orig_termios);
    new_termios = orig_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    write(STDOUT_FILENO, prompt, strlen(prompt));
    while (1) {
        char c;
        read(STDIN_FILENO, &c, 1);

        if (c == 4) { 
            write(STDOUT_FILENO, "\b \b", 3);
            printf("\nbb\n");
            return 0;
        }
        if (c == '\n') { // Enter key
            line[n] = '\0';
            write(STDOUT_FILENO, "\n", 1);
            break;
        }
        else if (c == 127 || c == '\b') { // Backspace
            if (n > 0) {
                n--;
                write(STDOUT_FILENO, "\b \b", 3);
            }
        }
        else if (c == 27) { // Escape sequence
            char seq[2];
            if (read(STDIN_FILENO, &seq, 2) == 2 && seq[0] == '[') {
                if (seq[1] == 'D') { // Left arrow
                    if (n > 0) {
                        write(STDOUT_FILENO, "\b", 1);
                        n--;
                    }
                }
                else if (seq[1] == 'C') { // Right arrow
                    if (n < strlen(line)) {
                        write(STDOUT_FILENO, &line[n], 1);
                        n++;
                    }
                }
            }
        }
        else { // Regular character
            if (n < sizline - 1) {
                line[n++] = c;
                write(STDOUT_FILENO, &c, 1);
            }
        }
    }

    // Restore original terminal attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);

    return n;
}
