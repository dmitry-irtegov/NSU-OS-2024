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
    	if(fclose(fd) == EOF) {
	    perror("can't close file");
	    exit(2);
	}
    }
}

int main()
{
    uid_t uid = getuid();
    uid_t euid = geteuid();
    printf("uid: %d\neuid: %d\n", uid, euid);
    opentry();
    setuid(uid);
    printf("setuid(uid) executed\n");
    uid = getuid();
    euid = geteuid();
    printf("uid: %d\neuid: %d\n", uid, euid);
    opentry();
    return 0;
}
