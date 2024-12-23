#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>

int count = 0;

void counter(int sig)
{
    count += 1;
    if(write(1, "\a", 1) == -1)
    {
        write(2, "Failed to write beep", 20);
        _exit(EXIT_FAILURE);
    }
}

void show_results(int sig)
{
    char output[32];
    int len = sprintf(output, "\nResulting in %d beeps\n", count);

    if(write(1, output, len) == -1)
    {
        write(2, "Failed to write results", 23);
        _exit(EXIT_FAILURE);
    }

    _exit(EXIT_SUCCESS);
}

int main()
{
    if(sigset(SIGINT, counter) == SIG_ERR)
    {
        perror("Failed to set signal for SIGINT");
        _exit(EXIT_FAILURE);
    }
    
    if(sigset(SIGQUIT, show_results) == SIG_ERR)
    {
        perror("Failed to set signal for SIGQUIT");
        _exit(EXIT_FAILURE);
    }

    while(1)
    {
        pause();
    }
}
