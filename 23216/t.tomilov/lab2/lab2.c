#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    if (putenv("TZ=PST8PDT") == -1){
        printf("Error: Couldn't get to the environment variable!\n");
        exit(-1);
    }
    time_t now;
    struct tm *sp;
    (void) time( &now );
    printf("%s", ctime( &now ) );
    exit(0);
}