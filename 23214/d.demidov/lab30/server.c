#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define BATCH_SIZE 6

char * path = "socksess";

int main() {
	struct sockaddr_un address;
	int socket_id = 0;
	int client_id;
	int cnt;
	char buffer[BATCH_SIZE];
	socket_id = socket(AF_UNIX, SOCK_STREAM, 0);
	if (socket_id == -1) {
		perror("socket creation error");
		exit(1);
	}
	memset( & address, 0, sizeof(address));
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, path, sizeof(address.sun_path));
	if (bind(socket_id, (struct sockaddr *)&address, sizeof(address)) == -1) {
		close(socket_id);
		perror("bind error");
		exit(1);
	}
	if (listen(socket_id, 1) == -1) {
		close(socket_id);
		unlink(path);
		perror("listen error");
		exit(1);
	}
	client_id = accept(socket_id, NULL, NULL);
	if (client_id == -1) {
		close(socket_id);
		unlink(path);
		perror("client_id error");
		exit(1);
	}
	while ((cnt = read(client_id, buffer, BATCH_SIZE)) > 0) {
		for (int i = 0; i < cnt; i++) {
			buffer[i] = toupper(buffer[i]);
		}
		printf("%s", buffer);
		fflush(stdout);
	}
	if (cnt == -1) {
		close(client_id);
		close(socket_id);
		unlink(path);
		perror("read error");
		exit(1);
	}
	printf("\n");
	close(client_id);
	close(socket_id);
	unlink(path);
	return 0;
}