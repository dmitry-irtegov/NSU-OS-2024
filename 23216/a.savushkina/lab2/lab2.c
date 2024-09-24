#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    int err = setenv("TZ", "America/Los_Angeles", 1);
    if (err == -1){
        perror("error in setenv");
        exit(1);
    }

    time_t now;
    
    if (time(&now) == (time_t)(-1)){
        perror("Current calendar time has not been encoded as time_t object");
        exit(1);
    }

    char* t = ctime(&now);
    if(t == NULL){
        perror("ctime error");
        exit(2);
    }
    
    printf("%s", t);

    exit(0);
}