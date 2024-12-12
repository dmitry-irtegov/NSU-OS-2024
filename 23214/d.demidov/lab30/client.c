#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

char * path = "sockssss";

int main() {
	int socket_id = 0;
	int server_id = 0;
	socket_id = socket(AF_UNIX, SOCK_STREAM, 0);
	if (socket_id == -1) {
		perror("socket error");
		exit(1);
	}
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path));
	server_id = connect(socket_id, (struct sockaddr*)&addr, sizeof(addr));
	if (server_id == -1) {
		close(socket_id);
		perror("connect error");
		exit(1);
	}
	char msg[] = "I love Os course its so EAsY!";
	if (write(socket_id, msg, strlen(msg)) == -1) {
		close(socket_id);
		perror("write error");
		exit(1);
	}
	close(socket_id);
	exit(0);
}