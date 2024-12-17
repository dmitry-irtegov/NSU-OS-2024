#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
	int fd;
	if ((fd = open("/tmp/lab8test", O_RDWR)) == -1) {
		perror("open error");
		exit(EXIT_FAILURE);
	}
	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	if (fcntl(fd, F_SETLK, &lock) == -1) {
		perror("setting lock error");
		exit(EXIT_FAILURE);
	}
	
	if (system("nano /tmp/lab8test") == -1) {
		perror("system error");
		exit(EXIT_FAILURE);
	}
	lock.l_type = F_UNLCK;
	if (fcntl(fd, F_SETLK, &lock) == -1) {
		perror("setting unlock error");
		exit(EXIT_FAILURE);
	}
	close(fd);
	exit(EXIT_SUCCESS);

}
