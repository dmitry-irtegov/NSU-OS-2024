#include <stdio.h>
#include <signal.h>
int count;
FILE *fptr;

main()
{
    count = 0;
    void sigcatch(int);
    fptr = fopen(OUTPUT, "w");
    signal(SIGINT, sigcatch);
    signal(SIGQUIT, sigcatch);
    while(true){
        wait();
    }
}


void sigcatch(int sig)
{
    char buf[40];
    signal(sig, SIG_IGN);
    if(sig == SIGINT){
        count++;
        write(1,"/a",1);
    }
    else if (sig == SIGQUIT) {
        sprintf(buf,"%d signals count\n", count);
        write(1, buf, strlen(buf));
        fclose(fptr);
        exit(1);
    }
    signal(sig, sigcatch);
}
