#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

struct termios old_tio;

void set_terminal_mode(struct termios *old_tio) {
    struct termios new_tio;
    
    if (tcgetattr(STDIN_FILENO, old_tio) != 0) {
        perror("tcgetattr failed");
        exit(EXIT_FAILURE);
    }
    new_tio = *old_tio;
    new_tio.c_lflag &= ~(ICANON);
    new_tio.c_cc[VMIN] = 1;
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

void handle_sigcont(int signo) {
    printf("\nReceived SIGCONT, restoring changed terminal state...\n");
    printf("Yes or no? (y/n): ");
    set_terminal_mode(&old_tio);
    fflush(stdout);
}
int main() {

    char answer;
    if (signal(SIGCONT, handle_sigcont) == SIG_ERR) {
        perror("signal(SIGCONT)");
        exit(EXIT_FAILURE);
    }
    set_terminal_mode(&old_tio);
    printf("Yes or no? (y/n): ");
    fflush(stdout);
    while (1)
    {
     if (read(STDIN_FILENO, &answer, 1) != 1) {
        restore_terminal_mode(&old_tio);
        if(errno == EINTR)
        {
            continue;
        }
        else
        {
            break;
        }
     }
     printf("\n");
     if (answer == 'y' || answer == 'Y') {
         printf("Yes!\n");
     } else if (answer == 'n' || answer == 'N') {
         printf("No!\n");
     } else {
         printf("Invalid response: %c\n", answer);
     }
     restore_terminal_mode(&old_tio);
     return 0;
    }
}