#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_LINE_LENGTH 40
#define CTRL_G "\a"
#define ENTER '\n'

struct termios original_termios;

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

void enable_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
       perror("tcgetattr");
       exit(EXIT_FAILURE);
    }
    atexit(disable_raw_mode);
    struct termios raw = original_termios;
    raw.c_lflag &= ~(ICANON | ECHO); 
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
       perror("tcsetattr");
       exit(EXIT_FAILURE);
    }
}

void beep() {
    if (write(STDOUT_FILENO, CTRL_G, 1) == -1) {
       perror("write");
       exit(EXIT_FAILURE);
    }
}

int main() {
    enable_raw_mode();
    
    char line[MAX_LINE_LENGTH + 1] = {0};
    int position = 0;
    char c;

    char ctrl_d = original_termios.c_cc[VEOF];
    char erase = original_termios.c_cc[VERASE];
    char kill = original_termios.c_cc[VKILL];
    char ctrl_w = original_termios.c_cc[VWERASE];
    printf("Write text (CTRL-D to exit):\n");

    while (1) {
        if (read(STDIN_FILENO, &c, 1) == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        if (c == ctrl_d && position == 0) {
            break;
        }
        if (c == ctrl_w) {
            while (position > 0 && line[position - 1] == ' ') {
                position--;
            }
            while (position > 0 && line[position - 1] != ' ') {
                position--;
            }
            while (position < MAX_LINE_LENGTH && line[position] == ' ') {
                position++;
            }
            line[position] = '\0';
            char buffer[MAX_LINE_LENGTH + 5];
            int len = snprintf(buffer, sizeof(buffer), "\r\033[K%s", line);
            if (len > 0) {
                if (write(STDOUT_FILENO, buffer, len) == -1) {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            }
            continue;
        }
        if (c == erase) {
            if (position > 0) {
                position--;
                line[position] = '\0';
                char buffer[MAX_LINE_LENGTH + 5];
                int len = snprintf(buffer, sizeof(buffer), "\r\033[K%s", line);
                if (len > 0) {
                    if (write(STDOUT_FILENO, buffer, len) == -1) {
                        perror("write");
                        exit(EXIT_FAILURE);
                    }
                }
            }
            continue;
        }
        if (c == kill) {
            position = 0;
            line[position] = '\0';
            const char *clear_line = "\r\033[K\r";
            if (write(STDOUT_FILENO, clear_line, strlen(clear_line)) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            continue;
        }
        if (position < MAX_LINE_LENGTH && c >= 32 && c <= 126) {
            line[position++] = c;
            line[position] = '\0';
            if (write(STDOUT_FILENO, &c, 1) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
        } else if (c < 32 || c > 126) {
            beep();
        }
        if (position >= MAX_LINE_LENGTH) {
            position = 0;
            const char *newline = "\n";
            if (write(STDOUT_FILENO, newline, 1) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }
        if (c == ENTER || c == '\r') {  
           const char *newline = "\n";
           if (write(STDOUT_FILENO, newline, 1) == -1) {
               perror("write");
               exit(EXIT_FAILURE);
           }
           line[position] = '\0';  
           memset(line, 0, sizeof(line));
           position = 0;
           continue;
        }
    }
    return 0;
}
