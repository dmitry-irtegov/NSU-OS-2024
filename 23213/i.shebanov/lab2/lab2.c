#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    setenv("TZ", "America/Los_Angeles", 1);

    time_t current_time;
    current_time = time(NULL);

    printf(asctime(localtime(&current_time)));

    exit(0);
}
