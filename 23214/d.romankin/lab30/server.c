#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

char* path = "socket";

int main() {
	
	struct sockaddr_un address;
	int s = 0;
	int client;
	ssize_t bytes_read;
	char recieved[5];
	s = socket(AF_UNIX, SOCK_STREAM, 0);
	if (s == -1) {
		perror("socket creation error");
		exit(EXIT_FAILURE);
	}
	
	memset(&address, 0, sizeof(address));
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, path, sizeof(address.sun_path) - 1);
	if (bind(s,(struct sockaddr*) &address, sizeof(address)) == -1){
		perror("bind error");
		exit(EXIT_FAILURE);
	}
	
	
	if (listen(s, 1) == -1) {
		close(s);
		unlink(path);
		perror("listen error");
		exit(EXIT_FAILURE);
	}
	client  = accept(s, NULL, NULL);
	if (client == -1) {
		close(s);
		unlink(path);
		perror("client error");
		exit(EXIT_FAILURE);
	}
	while ((bytes_read = read(client, recieved, 5)) > 0) {
		for (int i = 0; i < bytes_read; i++) {
			recieved[i] = toupper(recieved[i]);
		}
		write(1, recieved, bytes_read);
	}
	if (bytes_read == -1) {
		close(client);
		close(s);
		unlink(path);
		perror("read error");
		exit(EXIT_FAILURE);
	}
	printf("\n");
	close(client);
	close(s);
	unlink(path);
	exit(EXIT_SUCCESS);
}
