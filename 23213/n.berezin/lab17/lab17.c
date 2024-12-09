#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CTRL_D 4
#define CTRL_W 23

#define LINE_LENGTH 40

void fputs_stdout_exit_on_err(const char *s) {
    if (fputs(s, stdout) == EOF) {
        perror("Can't write");
        exit(1);
    }
}

void putchar_exit_on_err(int c) {
    int r = putchar(c);
    if (r == EOF) {
        perror("Can't write");
        exit(1);
    }
}

struct string {
    char *buf;
    int len;
    int cap;
};

struct string new_string() {
    return (struct string){.buf = NULL, .len = 0, .cap = 0};
}

void string_push(struct string *s, char c) {
    if (s->len == s->cap) {
        s->cap = s->cap * 2 + 4;
        char *new_buf = realloc(s->buf, s->cap);
        if (new_buf == NULL) {
            perror("Can't allocate memory");
            exit(1);
        }
        s->buf = new_buf;
    }
    s->buf[s->len] = c;
    s->len++;
}

void string_pop(struct string *s) {
    s->len--;
}

void string_back(struct string *s, int n) {
    s->len -= n;
}

int main() {
    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Stdin is not a terminal\n");
        exit(1);
    }

    struct termios tty;
    if (tcgetattr(STDIN_FILENO, &tty) != 0) {
        perror("Can't get attributes of stdin");
        exit(1);
    }

    cc_t erase = tty.c_cc[VERASE];
    cc_t kill = tty.c_cc[VKILL];

    struct termios saved_tty = tty;
    tty.c_lflag &= ~(ISIG | ICANON | ECHO);
    tty.c_cc[VMIN] = 1;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) != 0) {
        perror("Can't set terminal attributes");
        exit(1);
    }
    setbuf(stdout, NULL);

    struct string output_string = new_string();
    int pos = 0;
    char buf[LINE_LENGTH];

    while (1) {
        int ic = getchar();
        if (ic == EOF) {
            if (ferror(stdin)) {
                perror("Read error");
            } else {
                fprintf(stderr, "Unexpected EOF\n");
            }
            exit(1);
        }

        char c = (char)ic;

        if (c == erase) {
            if (pos > 0) {
                fputs_stdout_exit_on_err("\b \b");
                string_pop(&output_string);
                pos--;
            }
        } else if (c == kill) {
            while (pos > 0) {
                fputs_stdout_exit_on_err("\b \b");
                string_pop(&output_string);
                pos--;
            }
        } else if (c == CTRL_W) {
            int word_met = 0;
            while (pos > 0) {
                if (buf[pos - 1] != ' ') {
                    word_met = 1;
                } else if (word_met) {
                    break;
                }
                string_pop(&output_string);
                pos--;
                fputs_stdout_exit_on_err("\b \b");
            }
        } else if (c == CTRL_D) {
            if (pos == 0) {
                break;
            }
        } else if (c == '\n') {
            string_push(&output_string, '\n');
            pos = 0;
            putchar_exit_on_err('\n');
        } else if (32 <= c && c < 128) {
            string_push(&output_string, c);
            pos++;
            if (pos > LINE_LENGTH) {
                int last_word_pos = LINE_LENGTH;
                pos = 0;

                while (c != ' ' && last_word_pos > 0 && buf[last_word_pos - 1] != ' ') {
                    last_word_pos--;
                }

                if (c != ' ' && last_word_pos != 0) {
                    for (int i = last_word_pos; i < LINE_LENGTH; i++) {
                        fputs_stdout_exit_on_err("\b \b");
                    }
                    putchar_exit_on_err('\n');

                    // LINE_LENGTH - last_word_pos > 0 => pos will be < LINE_LENGTH
                    for (; last_word_pos < LINE_LENGTH; last_word_pos++) {
                        char current = buf[last_word_pos];
                        buf[pos] = current;
                        pos++;
                        putchar_exit_on_err(current);
                    }
                } else {
                    putchar_exit_on_err('\n');
                }

                putchar_exit_on_err(c);
                pos++;
            } else {
                buf[pos - 1] = c;
                putchar_exit_on_err(c);
            }
        } else {
            putchar_exit_on_err('\a');
        }
    }

    string_push(&output_string, '\0');
    fputs(output_string.buf, stdout);

    if (tcsetattr(STDIN_FILENO, TCSANOW, &saved_tty) != 0) {
        perror("Can't restore terminal attributes");
        exit(1);
    }

    exit(0);
}
