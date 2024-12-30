#include<stdlib.h>
#include<stdio.h>
#include<termios.h>
#include<signal.h>
#include<unistd.h>
#include<fcntl.h>


int main()
{
        int fd = open("/dev/tty", O_RDWR);
        struct termios tty, otty;
        tcgetattr(fd, &tty);
        otty = tty;

        tty.c_lflag &= ~(ICANON | ISIG);
        tty.c_cc[VMIN] = 1;
        tty.c_cc[VTIME] = 0;

        tcsetattr(fd, TCSANOW, &tty);

        setbuf(stdout, (char*)NULL);

        printf("Will you answer my question? [y/n]: ");

        char ans[1];
        if(read(fd, ans, 1) == -1)
        {
            perror("Failed to read answer");
            tcsetattr(fd, TCSAFLUSH, &otty);
            exit(EXIT_FAILURE);
        }
        
        if(ans[0] == 'y')
        {
            printf("\nGood\n");
        }
        else 
        {
            printf("\nOkay\n");
        }
        
        tcsetattr(fd, TCSAFLUSH, &otty);
        exit(EXIT_SUCCESS);
}
