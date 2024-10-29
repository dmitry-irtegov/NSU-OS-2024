#include <stdio.h>
#include <time.h>
#include <stdlib.h> 

int main(){
    time_t now;

    if (putenv("TZ=America/Los_Angeles") != 0){
        perror("Failed to get environment variable!");
        exit(EXIT_FAILURE);
    }
    
    if (time(&now) == -1){
        perror("Failed to get time!");
        exit(EXIT_FAILURE);
    }

    char *california_time = ctime(&now);

    if (california_time == NULL){
        perror("Failed to return time!");
        exit(EXIT_FAILURE);
    }

    printf("%s", california_time);

    exit(EXIT_SUCCESS);
    
}