#include <sys/types.h>
#include <time.h>
#include <stdio.h>

int main(){
    time_t cur_time = time(NULL);

    if (cur_time == -1) {
        printf("Time error");
        return (1);
    }
    
    time_t calif_time = cur_time - 50400;
    printf("%s", ctime(&calif_time));
    return(0);
}