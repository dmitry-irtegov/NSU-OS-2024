#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>


#define BUFFER_SIZE BUFSIZ
char* path = "socket";

int main() {
	struct sockaddr_un addr;
	int server = 0;
	int client;
	ssize_t bytes_read;
	char recieved[BUFFER_SIZE];
	server = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server == -1) {
        perror("error creating socket");
		exit(EXIT_FAILURE);
	}
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
	if (bind(server,(struct sockaddr*) &addr, sizeof(addr)) == -1){
		close(server);
		perror("bind error");
		exit(EXIT_FAILURE);
	}
	if (listen(server, 1) == -1) {
		close(server);
		unlink(path);
		perror("listen error");
		exit(EXIT_FAILURE);
	}
	client  = accept(server, NULL, NULL);
	if (client == -1) {
		close(server);
		unlink(path);
		perror("client error");
		exit(EXIT_FAILURE);
	}
	while ((bytes_read = read(client, recieved, BUFFER_SIZE)) > 0) {
		for (int i = 0; i < bytes_read; i++) {
			recieved[i] = toupper(recieved[i]);
		}
		write(1, recieved, bytes_read);
	}
	if (bytes_read == -1) {
		close(client);
		close(server);
		unlink(path);
		perror("read error");
		exit(EXIT_FAILURE);
	}
	close(client);
	close(server);
	unlink(path);
	exit(EXIT_SUCCESS);
}