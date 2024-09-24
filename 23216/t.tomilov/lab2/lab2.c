#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    if (putenv("TZ=America/Los_Angeles") == -1) {
        perror("Error: Couldn't get the environment variable!");
        exit(EXIT_FAILURE);
    }
    time_t now;
    if (time(&now) == -1) {
        perror("Error: Failed to get the system time!");
        exit(EXIT_FAILURE);
    }
    char * california_time = ctime(&now);
    if (california_time == NULL){
        perror("Error: can`t return time!");
        exit(EXIT_FAILURE);
    }
    printf("%s", california_time);
    exit(EXIT_SUCCESS);
}