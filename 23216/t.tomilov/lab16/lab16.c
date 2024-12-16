#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

int main(){
    if (!isatty(STDIN_FILENO)){
        perror("ERROR: chang inout/output to standart(terminal)!");
        exit(EXIT_FAILURE);
    }
    
    struct termios oldt, newt;

    if (tcgetattr(STDIN_FILENO, &oldt) == -1){
        perror("ERROR: can`t get terminal`s settings!");
        exit(EXIT_FAILURE);
    }
    newt = oldt;
    newt.c_lflag &= ~(ICANON);
    newt.c_cc[VMIN] = 1;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) == -1){
        perror("ERROR: can`t set terminal`s settings");
        exit(EXIT_FAILURE);
    }

    char num = 0;
    printf("Write number 0 - 9: ");
    if (scanf("%c", &num) == 0){
        perror("ERROR: input error!");
        if (tcsetattr(STDIN_FILENO, TCSANOW, &oldt) == -1){
            perror("ERROR: can`t set terminal`s settings");
        }
        exit(EXIT_FAILURE);
    }
    printf("\nOKAY!\n");
    
    if (tcsetattr(STDIN_FILENO, TCSANOW, &oldt) == -1){
        perror("ERROR: can`t set terminal`s settings");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}