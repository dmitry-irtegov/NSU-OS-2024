#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>

int main() {
    char ch, *text = "Input your favorite symbol";
    int fd;
    struct termios tty, savetty;

    fd = open("/dev/tty", O_RDONLY);
    if (fd == -1) {
        perror("open error");
        exit(1);
    }

    if (tcgetattr(fd, &tty)) {
        perror("tcgetattr error");
        exit(1);
    }
    
    
    savetty = tty;
    tty.c_lflag &= ~(ISIG | ICANON);
    tty.c_cc[VMIN] = 1;
    if (tcsetattr(fd, TCSAFLUSH, &tty) == -1) {
        perror("tcsetattr error1");
        exit(1);
    }
    
    
    printf("Answer the question:\n\n%s\n", text);
    if (read(fd, &ch, 1) == -1) {
        perror("read error");
        exit(1);
    }
    
    printf("\n");
    
    if (tcsetattr(fd, TCSAFLUSH, &savetty) == -1) {
        perror("tcsetattr error2");
        exit(1);
    }

    printf("Your favorite symbol - %c\n", ch);

    
    
    exit(0);
}