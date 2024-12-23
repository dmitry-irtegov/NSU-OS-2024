#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <signal.h>
#include "shell.h"

void disable_job_control() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~ISIG; // Отключаем обработку сигналов терминала
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void able_job_control() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ISIG; // Отключаем обработку сигналов терминала
    signal(SIGTSTP, sigSTOP);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

int promptline(char* prompt, char* line, int sizline) {
    int n = 0;

    reset_terminal();
    write(STDOUT_FILENO, prompt, strlen(prompt));
    while (1) {
        char c;

        disable_job_control();
        read(STDIN_FILENO, &c, 1);
        able_job_control();

        if (c == 4) { 
            write(STDOUT_FILENO, "\b \b", 3);
            printf("\nbb\n");
            return -1;
        }
        if (c == '\n') { // Enter key
            line[n] = '\0';
            write(STDOUT_FILENO, "\n", 1);

            break;
        }
        else { // Regular character
            if (n < sizline - 1) {
                line[n++] = c;
                //write(STDOUT_FILENO, &c, 1);
            }
        }
    }

    return n;
}
