#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stddef.h"
#include "unistd.h"
#include <ctype.h>

#include <sys/socket.h>
#include <sys/types.h> 
#include <sys/un.h>
#include "sys/syscall.h"



int main() {
	int socket_fd = 0;
	struct sockaddr_un address;
	socklen_t addrlen = sizeof(address);
	int cntOfBytesGetted = 0;
	char str[1024], path[] = "/tmp/lab31_socket";




	socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (socket_fd == -1) {
		perror("socket error");
		exit(1);
	}

	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, path);

	if (connect(socket_fd, (struct sockaddr*)&address, addrlen) == -1) {
		perror("connect error");
		exit(1);
	}

	for (;;) {
		cntOfBytesGetted = read(0, str, 1023);
		if (cntOfBytesGetted == -1) {
			perror("read error");
			exit(1);
		}

		if (cntOfBytesGetted <= 0) {
			break;
		}

		if (write(socket_fd, str, cntOfBytesGetted) == -1) {
			perror("write error");
			exit(1);
		}
	}


	if (close(socket_fd) == -1) {
		perror("close error");
		exit(1);
	}
	exit(0);
}