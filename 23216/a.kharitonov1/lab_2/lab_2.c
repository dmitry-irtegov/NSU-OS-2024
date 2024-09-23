#include <stdio.h>
#include <time.h>
#include <stdlib.h>

main() {
    time_t now;

    if (putenv("TZ=America/Los_Angeles") == -1) {
        perror("failed putenv");
        exit(EXIT_FAILURE);
    }

    if (time(&now) == 0) {
        perror("failed time");
        exit(EXIT_FAILURE);
    }

    printf("%s", ctime(&now));

    exit(EXIT_SUCCESS);
}