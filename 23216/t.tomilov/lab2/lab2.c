#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>

int main() {
    if (putenv("TZ=America/Los_Angeles") == -1) {
        perror("Error: Couldn't get the environment variable!\n");
        exit(-1);
    }
    time_t now;
    struct tm *sp;
    if (time(&now) == 0) {
        perror("Error: Failed to get the system time!\n");
        exit(-1);
    }
    printf("%s", ctime(&now));
    exit(0);
}