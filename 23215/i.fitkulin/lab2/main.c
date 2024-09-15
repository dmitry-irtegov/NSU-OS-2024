#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

void main()
{
    setenv("TZ", "America/Los_Angeles", 1);
    time_t now;
    (void) time( &now );
    printf("%s", ctime( &now ) );
    exit(0);
}

