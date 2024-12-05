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
	s = socket(AF_UNIX, SOCK_STREAM, 0);
	if (s == -1) {
        perror("error creating socket");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
	server = connect(s,(struct sockaddr*)&addr, sizeof(addr));
	if (server == -1) {
		close(s);
		perror("connection error");
		exit(EXIT_FAILURE);
	}
	char text[BUFSIZ] = "Test String\n";
	if (write(s, text, strlen(text) + 1) == -1) {
		close(s);
		perror("write error");
		exit(EXIT_FAILURE);
	}
	close(s);
	exit(EXIT_SUCCESS);
}