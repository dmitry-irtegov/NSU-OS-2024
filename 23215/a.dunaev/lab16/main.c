#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    struct termios old, new;
    
    printf("Enter a single character: ");
        fflush(stdout); 
        
        if (tcgetattr(STDIN_FILENO, &old)){                             // E_BAD_F E_NO_TTY
                perror("Bad parameters");
                exit(1);
        }
        
    new = old;
        new.c_lflag &= ~ICANON;
        new.c_cc[VMIN] = 1;
        new.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new)){        // E_BAD_F E_INTR E_IN_VAL E_NO_TTY  E_IO
        perror("Bad parameters or error in I/O");
                exit(1);
    }

    char response;
    read(STDIN_FILENO, &response, 1);
        if (tcsetattr(STDIN_FILENO, TCSANOW, &old)){
                perror("Bad parameters or error in I/O");
                exit(1);
        }

    printf("\nYou entered: %c\n", response);

    return 0;
}
