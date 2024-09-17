#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

void main() {
    time_t now;

    /* As follows from man, timezone getting from
    TZ environment variable by tzset() function.

    This value used by ctime and localtime to
    convert timestamp into correct for current
    timezone time.
    */

    if (putenv("TZ=America/Los_Angeles") != 0) {
        exit(1);
    }

    if (time(&now) == NULL) {
        exit(2);
    }

    printf("%s", ctime(&now));

    exit(0);
}