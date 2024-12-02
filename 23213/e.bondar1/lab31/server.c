#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>

#define BACKlOG 100
#define BATCH_SIZE 1024

char* socket_path = "./socket";

void handle_sigint(int sig) {
	unlink(socket_path);
	_exit(EXIT_SUCCESS);
}

int create_server_socket(void) {
	int sock;
	struct sockaddr_un addr;

	if((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("Failed to create socket");
		exit(EXIT_FAILURE);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

	if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("Failed to bind()");
		exit(EXIT_FAILURE);
	} 

	if (listen(sock, BACKlOG) == -1) {
		perror("listen failed");
		unlink(socket_path);
		exit(EXIT_FAILURE);
	}

	printf("Server Socket registered as listening!\n");

	return sock;
}

void accept_new_client(int server_sock, fd_set* active_fds, int* max_fd) {
	int client_sock = accept(server_sock, NULL, NULL);
	if (client_sock == -1) {
		perror("Failed to accept()");
	} else {
		FD_SET(client_sock, active_fds);
		if (client_sock > *max_fd) *max_fd = client_sock;
	}
}

void handle_client(int client_sock, fd_set* active_fds) {
	char buffer[BATCH_SIZE];
	ssize_t read_bytes = read(client_sock, buffer, sizeof(buffer));

	if (read_bytes > 0) {
		for (int i = 0; i < read_bytes; i++) {
			putchar(toupper(buffer[i]));
		}
	} else if (read_bytes == 0) {
		close(client_sock);
		FD_CLR(client_sock, active_fds);
	} else {
		perror("read() from client socket failed");
		close(client_sock);
		FD_CLR(client_sock, active_fds);
	}
}

int main(void) {
	int max_fd;
	fd_set active_fds, read_fds;
	FD_ZERO(&active_fds);

	int sock = create_server_socket();
	FD_SET(sock, &active_fds);
	max_fd = sock;

	signal(SIGINT, handle_sigint);

	while(1) {
		read_fds = active_fds;

		int selected_fds = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
		if (selected_fds == -1) {
			perror("select() failed");
			unlink(socket_path);
			exit(EXIT_FAILURE);
		}

		for (int i = 0, checked = 0; i <= max_fd && checked < selected_fds; i++) {
			if (FD_ISSET(i, &read_fds)) {
				checked++;
				if (i == sock) {
					accept_new_client(sock, &active_fds, &max_fd);
				} else {
					handle_client(i, &active_fds);
				}
			} 
		} 
	}
}
