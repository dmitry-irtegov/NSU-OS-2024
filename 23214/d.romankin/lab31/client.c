#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>


char* path = "socket";

int main() {
	int s = 0;
	int server = 0;
	ssize_t bytes_read;
	s = socket(AF_UNIX, SOCK_STREAM, 0);
	if (s == -1) {
		perror("socket error");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_un address;
	memset(&address, 0, sizeof(address));
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, path, sizeof(address.sun_path) - 1);
	server = connect(s,(struct sockaddr*)&address, sizeof(address));
	if (server == -1) {
		close(s);
		perror("connect error");
		exit(EXIT_FAILURE);
	}
	char recieved[BUFSIZ];
	ssize_t bytes_written;
	while ((bytes_read = read(0, recieved, BUFSIZ)) > 0) {
		if ((bytes_written = write(s, recieved, bytes_read)) == -1) {
			close(s);
			perror("write error");
			exit(EXIT_FAILURE);
		}
		
	}
	if (bytes_read == -1) {
		close(s);
		perror("read error");
		exit(EXIT_FAILURE);
	}
	close(s);
	exit(EXIT_SUCCESS);
}
