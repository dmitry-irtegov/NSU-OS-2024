#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

void opentry()
{
    FILE* fd = fopen("testfile", "r");
    if (fd == NULL) {
        perror("can't open the file");
        exit(1);
    } else {
	printf("file opened successfully\n");
    }
}

int main()
{
    uid_t uid = getuid();
    uid_t euid = geteuid();
    printf("uid: %d\neuid: %d\n", uid, euid);
    opentry();
    if(setuid(uid) == -1) {
    	perror("setuid failed");
    } else {
        printf("setuid(uid) executed\n");
    }
    uid = getuid();
    euid = geteuid();
    printf("uid: %d\neuid: %d\n", uid, euid);
    opentry();
    return 0;
}
