#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>


int main()
{
    char input;
    int fd;
    struct termios tty, savetty;
    fd = fileno(stdin);
    
    if (isatty(fd) == 0) {
        fprintf(stderr, "Stdin not terminal");
        exit(1);
    }
    if (tcgetattr(fd, &tty) == -1) {
	perror("Couldn`t get attributes");
	exit(-1);
    }
    savetty = tty;
    tty.c_lflag &= ~(ISIG | ICANON);
    tty.c_cc[VMIN] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) == -1) {
        perror("Couldn`t set attributes");
        exit(-1);
    }

    setbuf(stdout,(char*) NULL);
    printf("Which way the compass arrow would point if we look with it towards the rising sun? ");
    input = getchar();
    
    printf("\n");

    if (input == 'E' || input == 'e') {
        printf("Yes, that is right\n");
    }
    else 
    {
        printf("No, different direction\n");
    }
    
    if (tcsetattr(fd, TCSANOW, &savetty) == -1) {
        perror("Couldn`t set attributes");
        exit(-1);
    }
    exit(0);
}
