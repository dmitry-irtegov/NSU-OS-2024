#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
    if(putenv("TZ=America/Los_Angeles") != 0){
        perror("error changing environment variable.");
        return 1;
    }

    tzset();
    time_t now = time(NULL);
    if (now == -1) {
        perror("Error getting time");
        return 1;
    }
    char * outputTime = ctime(&now);  
     if (outputTime == NULL) {
        perror("Error converting time");
        return 1;
    }  
    printf("%s", outputTime);
    return 0;
}