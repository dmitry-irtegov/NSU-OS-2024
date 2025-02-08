#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
int main(){
    struct termios oldt, newt;
    if (tcgetattr(STDIN_FILENO, &oldt)  != 0){
        perror("error of getting terminal settings");
        exit(EXIT_FAILURE);
    }
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); 
    newt.c_cc[VMIN] = 1;
    if(tcsetattr(STDIN_FILENO, TCSANOW, &newt) != 0){
        perror("error of changing terminal settings");
        exit(EXIT_FAILURE);
    }

    printf("Start?\n");

    char input = getchar();
    if(tcsetattr(STDIN_FILENO, TCSANOW, &oldt) != 0){
        perror("error of getting terminal settings");
        exit(EXIT_FAILURE);
    }
    printf("your choice: %c \n", input);
    exit(EXIT_SUCCESS);
}
