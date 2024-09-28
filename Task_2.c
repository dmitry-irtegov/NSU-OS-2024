#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
    if(putenv("TZ=America/Los_Angeles") != 0){
        perror("error changing environment variable.");
        exit(EXIT_FAILURE);
    }

    time_t now = time(NULL);
    if (now == -1) {
        perror("Error getting time");
        exit(EXIT_FAILURE);
    }
    char * outputTime = ctime(&now);  
     if (outputTime == NULL) {
        perror("Error converting time");
        exit(EXIT_FAILURE);
    }  
    printf("%s", outputTime);
    exit(EXIT_SUCCESS);
}