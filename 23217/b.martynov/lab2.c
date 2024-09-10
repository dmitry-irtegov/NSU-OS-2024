#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
extern char* tzname[];

int main()
{
    //setenv("TZ", "PST8", 1);
    setenv("TZ", "America/Los_Angeles", 1);
    tzset();
    time_t now;
    struct tm* sp;

    (void)time(&now);

    printf("%s", ctime(&now));

    sp = localtime(&now);
    printf("%d/%d/%02d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year, sp->tm_hour,
        sp->tm_min, tzname[sp->tm_isdst]);

    exit(0);

    return 0;
}


// // The first solution
// #include <sys/types.h>
// #include <stdio.h>
// #include <time.h>
// #include <stdlib.h>
// extern char* tzname[];

// int main()
// {
//     time_t now;
//     struct tm* sp;

//     (void)time(&now);

//     // time -= 8 hours
//     time_t time_California = -8 * 60 * 60;
//     now += time_California;

//     // print UTC time (-8 hours)
//     printf("%s", asctime(gmtime(&now)));


//     sp = gmtime(&now);
//     printf("%d/%d/%02d %d:%02d %s\n",
//         sp->tm_mon + 1, sp->tm_mday,
//         sp->tm_year, sp->tm_hour,
//         sp->tm_min, tzname[sp->tm_isdst]);

//     exit(0);

//     return 0;
// }


// // INFO
// struct tm
// {
//     int tm_sec;   // seconds after the minute - [0, 60] including leap second
//     int tm_min;   // minutes after the hour - [0, 59]
//     int tm_hour;  // hours since midnight - [0, 23]
//     int tm_mday;  // day of the month - [1, 31]
//     int tm_mon;   // months since January - [0, 11]
//     int tm_year;  // years since 1900
//     int tm_wday;  // days since Sunday - [0, 6]
//     int tm_yday;  // days since January 1 - [0, 365]
//     int tm_isdst; // daylight savings time flag
// };