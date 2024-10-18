#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>  // 为了使用 _putenv()，不需要再声明 setenv()
#include <string.h>  // 为了使用 tzname


int main() {
    struct tm *local_tm, *gmt_tm;
    time_t now;
    char *ctime_str, *asctime_str;

    // 获取当前时间
    time(&now);

    // 使用 ctime() 打印当前时间（UTC格式）
    ctime_str = ctime(&now);
    printf("Current UTC time (ctime): %s", ctime_str);

    // 使用 localtime() 转换为本地时间
    local_tm = localtime(&now);
    printf("Local time (localtime): %d/%d/%02d %d:%02d:%02d\n",
           local_tm->tm_mon + 1,
           local_tm->tm_mday,
           local_tm->tm_year + 1900,
           local_tm->tm_hour,
           local_tm->tm_min,
           local_tm->tm_sec);

    // 使用 gmtime() 转换为UTC时间
    gmt_tm = gmtime(&now);
    printf("GMT time (gmtime): %d/%d/%02d %d:%02d:%02d\n",
           gmt_tm->tm_mon + 1,
           gmt_tm->tm_mday,
           gmt_tm->tm_year + 1900,
           gmt_tm->tm_hour,
           gmt_tm->tm_min,
           gmt_tm->tm_sec);

    // 使用 asctime() 将 localtime 转换为字符串并打印
    asctime_str = asctime(local_tm);
    printf("Local time (asctime): %s", asctime_str);

    printf("Timezone: %s\n", tzname[local_tm->tm_isdst]);  // 判断是否使用夏令时

    // 设置并打印加利福尼亚州时间（PST/PDT）
#ifdef _WIN32
    _putenv("TZ=PST8PDT");  // Windows上使用 _putenv 设置时区
#else
    setenv("TZ", "PST8PDT", 1);  // Linux/Unix上使用 setenv
    setenv("TZxcv", "PSxcvbcvvbbT8PDT", 1);  // Linux/Unix上使用 setenv

#endif
    tzset();
    local_tm = localtime(&now);
    printf("California time (PST/PDT): %d/%d/%02d %d:%02d:%02d %s\n",
           local_tm->tm_mon + 1,
           local_tm->tm_mday,
           local_tm->tm_year + 1900,
           local_tm->tm_hour,
           local_tm->tm_min,
           local_tm->tm_sec,
           tzname[local_tm->tm_isdst]);

    return 0;
}
