#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

extern char* tzname[];

int main() {
	time_t now;
	struct tm* sp;
	
	time_t for_err = time(&now);
	
	if (for_err == -1) {
		printf("time() error: -1\n");
		exit(-1);
	}

	int res_putenv = putenv("TZ=America/Los_Angeles");

	if (res_putenv != 0) {
		printf("putenv() error: %d\n", res_putenv);
		exit(-1);
	}

	char* time_ctime = NULL;
	time_ctime = ctime(&now);

	if (time_ctime == NULL) {
		printf("ctime() error: NULL\n");
		exit(-1);
	}

	else
		printf("%s", time_ctime);
	
	sp = localtime(&now);

	if (sp == NULL) {
		printf("localtime() error: NULL\n");
		exit(-1);
	}

	else {
		printf("%d/%d/%02d %d:%02d %s\n",
			sp->tm_mon + 1, sp->tm_mday,
			sp->tm_year + 1900, sp->tm_hour,
			sp->tm_min, tzname[sp->tm_isdst]);
	}

	exit(0);
}
