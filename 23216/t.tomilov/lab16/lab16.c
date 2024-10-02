#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

int terminalMode(int fl) {
    struct termios newt;
    if (tcgetattr(STDIN_FILENO, &newt) == -1){
        perror("ERROR: can`t get terminal`s settings!");
        return -1;
    }
    if (fl == 1) {
        newt.c_lflag &= ~(ICANON);
    }
    else {
        newt.c_lflag |= (ICANON);
    }
    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) == -1){
        perror("ERROR: can`t set terminal`s settings");
        return -1;
    }
    return 0;
}

int main(){
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
        exit(EXIT_FAILURE);
    }
    printf("\nOKAY!\n");
    
    if (tcsetattr(STDIN_FILENO, TCSANOW, &oldt) == -1){
        perror("ERROR: can`t set terminal`s settings");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}