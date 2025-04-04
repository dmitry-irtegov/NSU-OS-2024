#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#define MAX_LEN 40

int ttyfd = STDIN_FILENO;
struct termios prev_tty;

void exit_tcsetattr() {
    if (tcsetattr(ttyfd, TCSAFLUSH, &prev_tty) == -1) {
        printf("tcsetattr() failed.\n");
    }
}

void print_new_line(char* line) {
    char buffer[MAX_LEN + 5];
    int bytes_write = snprintf(buffer, sizeof(buffer), "\r\033[K%s", line);

    if (bytes_write > 0) {
        if (write(STDOUT_FILENO, buffer, bytes_write) == -1) {
            printf("write() failed.\n");
            exit(-1);
        }
    }
}

int main() {
    if (tcgetattr(ttyfd, &prev_tty) == -1) {
        printf("tcgetattr() failed.\n");
        exit(-1);
    }

    struct termios now_tty = prev_tty;
    now_tty.c_lflag &= ~(ICANON | ECHO);
    now_tty.c_cc[VMIN] = 1;
    now_tty.c_cc[VTIME] = 0;

    if (tcsetattr(ttyfd, TCSAFLUSH, &now_tty) == -1) {
        printf("tcsetattr() failed.\n");
        exit(-1);
    }

    if (atexit(exit_tcsetattr) != 0) {
        printf("atexit() failed.\n");

        if (tcsetattr(ttyfd, TCSAFLUSH, &prev_tty) == -1) {
            printf("tcsetattr() failed.\n");
        }

        exit(-1);
    }

    char symbol;

    char buffer[MAX_LEN + 1];
    memset(buffer, 0, sizeof(buffer));

    int now_indx = 0;

    while (1) {
        if (read(STDIN_FILENO, &symbol, 1) == -1) {
            printf("read() failed.\n");
            exit(-1);
        }

        if (symbol == prev_tty.c_cc[VERASE]) {
            if (now_indx > 0) {
                buffer[--now_indx] = 0;
                print_new_line(buffer);
            }

            continue;
        }

        if (symbol == prev_tty.c_cc[VKILL]) {
            now_indx = 0;
            buffer[now_indx] = 0;

            char* new_buffer = "\r\033[K\r";
            if (write(STDOUT_FILENO, new_buffer, strlen(new_buffer)) == -1) {
                printf("write() failed.\n");
                exit(-1);
            }

            continue;
        }

        if (symbol == prev_tty.c_cc[VWERASE]) {
            while (now_indx > 0 && buffer[now_indx - 1] == ' ') {
                now_indx--;
            }

            while (now_indx > 0 && buffer[now_indx - 1] != ' ') {
                now_indx--;
            }

            while (now_indx < MAX_LEN && buffer[now_indx] == ' ') { // for empty line
                now_indx++;
            }

            buffer[now_indx] = 0;
            print_new_line(buffer);

            continue;
        }

        if (symbol == prev_tty.c_cc[VEOF] && now_indx == 0) {
            break;
        }

        if (symbol == '\n') {
            if (write(STDOUT_FILENO, &symbol, 1) == -1) {
                printf("write() failed.\n");
                exit(-1);
            }

            now_indx = 0;
            memset(buffer, 0, sizeof(buffer));

            continue;
        }

        if (now_indx == MAX_LEN) {
            buffer[now_indx] = 0;

            while (now_indx > 0 && buffer[now_indx - 1] != ' ') {
                now_indx--;
            }

            if (now_indx != 0) {
                char mem_symbol = buffer[now_indx];
                buffer[now_indx] = 0;
                print_new_line(buffer);

                if (write(STDOUT_FILENO, "\n", 1) == -1) {
                    printf("write() failed.\n");
                    exit(-1);
                }

                buffer[now_indx] = mem_symbol;
                memmove(buffer, buffer + now_indx, MAX_LEN - now_indx);

                if (write(STDOUT_FILENO, buffer, MAX_LEN - now_indx) == -1) {
                    printf("write() failed.\n");
                    exit(-1);
                }

                now_indx = MAX_LEN - now_indx;
            }

            else {
                if (write(STDOUT_FILENO, "\n", 1) == -1) {
                    printf("write() failed.\n");
                    exit(-1);
                }

                if (write(STDOUT_FILENO, &symbol, 1) == -1) {
                    printf("write() failed.\n");
                    exit(-1);
                }

                buffer[0] = symbol;
                now_indx = 1;
            }

            continue;
        }

        if (symbol >= 32 && symbol <= 126) {
            buffer[now_indx++] = symbol;
            buffer[now_indx] = 0;

            if (write(STDOUT_FILENO, &symbol, 1) == -1) {
                printf("write() failed.\n");
                exit(-1);
            }
        }

        else if (write(STDOUT_FILENO, "\a", 1) == -1) {
            printf("write() failed.\n");
            exit(-1);
        }
    }

    exit(0);
}
