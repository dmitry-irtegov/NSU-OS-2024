#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <signal.h>


char* path = "socket";

void sighandler() {
	unlink(path);
	exit(EXIT_SUCCESS);
}

int main() {
	struct sockaddr_un address;
	struct pollfd fds[SOMAXCONN + 1];
	int s = 0;
	int client;
	ssize_t bytes_read;
	char recieved[BUFSIZ];
	s = socket(AF_UNIX, SOCK_STREAM, 0);
	if (s == -1) {
		perror("socket creation error");
		exit(EXIT_FAILURE);
	}
	memset(&address, 0, sizeof(address));
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, path, sizeof(address.sun_path) - 1);
	memset(fds, 0, sizeof(fds));
	fds[0].fd = s;
	fds[0].events = POLLIN;
	nfds_t clients_num = 0;
	if (bind(s,(struct sockaddr*) &address, sizeof(address)) == -1){
		perror("bind error");
		exit(EXIT_FAILURE);
	}
	if (signal(SIGINT, sighandler) == SIG_ERR) {
		close(s);
		unlink(path);
		perror("sighandler error");
		exit(EXIT_FAILURE);
	}
	if (listen(s, SOMAXCONN) == -1) {
		close(s);
		unlink(path);
		perror("listen error");
		exit(EXIT_FAILURE);
	}
	while (1) {

		if (fds[0].revents & POLLIN) {
			client  = accept(s, NULL, NULL);
			
			if (client == -1) {
				close(s);
				unlink(path);
				perror("accept error");
				exit(EXIT_FAILURE);
			}

			printf("client has been connected with descriptor = %d\n", client);
			
			clients_num++;

			if (clients_num == SOMAXCONN) {
				fds[0].events = 0;
			}

			fds[clients_num].fd = client;
			fds[clients_num].events = POLLIN;
			
			
			
		}
		if (poll(fds, clients_num + 1, -1) == -1) {
			close(s);
			unlink(path);
			perror("poll error");
			exit(EXIT_FAILURE);
		}
		

		for (int i = 1; i <= clients_num; i++) {
			if (fds[i].fd == 0) {
				continue;
			}
			if (fds[i].revents & POLLIN) {
				if ((bytes_read = read (fds[i].fd, recieved, BUFSIZ)) > 0) {
					for (ssize_t j = 0; j <= bytes_read; j++) {
						recieved[j] = toupper(recieved[j]);
					}
					write(1, recieved, bytes_read);
				}
				else {
					if (bytes_read == 0) {
						close(fds[i].fd);
						fds[i] = fds[clients_num];
						fds[clients_num].fd = 0;
						fds[clients_num].events = 0;
						fds[0].events = POLLIN;
						clients_num--;
					}
					
					if  (bytes_read == -1) {
						close(s);
						unlink(path);
						perror("read error");
						exit(EXIT_FAILURE);
					}
					
					
				}

			}
		}
		
	}
	exit(EXIT_SUCCESS);
}
