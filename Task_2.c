#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
    if(putenv("TZ=America/Los_Angeles") != 0){
        perror("error changing environment variable");
        return 1;
    }
    tzset();
    time_t now = time(NULL);
    char * outputTime = ctime(&now);    
    printf("%s", outputTime);
}