#include <stdio.h>
#include <time.h>
#include <stdlib.h> 

int main()
{
    time_t now;

    if (putenv("TZ=America/Los_Angeles")){
        perror("failed to putenv");
        exit(1);
    }
    
    if (time(&now)){
        perror("failed to get time");
        exit(1);
    }

    printf(asctime(localtime(&now)));

    return 0;
}