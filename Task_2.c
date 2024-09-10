#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
    putenv("TZ=America/Los_Angeles");
    tzset();
    time_t now = time(NULL);
    struct tm * timeCalifornia = localtime(&now);
    printf("date in Californa: %02d.%02d.%04d, time in Californa: %02d:%02d:%02d/n", timeCalifornia->tm_mday, timeCalifornia->tm_mon + 1, timeCalifornia->tm_year + 1900, 
    timeCalifornia->tm_hour, timeCalifornia->tm_min, timeCalifornia->tm_sec);
}