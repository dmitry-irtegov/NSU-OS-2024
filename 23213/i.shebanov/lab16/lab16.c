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

        printf("Will you answer my question? [y/n]\n");

        char ans[1];
        if(read(fd, ans, 1) == -1)
        {
            perror("failed to read answer");
            tcsetattr(fd, TCSANOW, &otty);
            exit(EXIT_FAILURE);
        }
        
        if(ans[0] == 'y')
        {
            printf("Good\n");
        }
        else 
        {
            printf("Okay\n");
        }
        
        tcsetattr(fd, TCSANOW, &otty);
        exit(EXIT_SUCCESS);
}
