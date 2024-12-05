#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>

#define BUFFSIZE 1024

int main() {
        char* socketPath = "./socket_file";

	int serverSocket;
	serverSocket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (serverSocket == -1) {
		perror("Socket creation failed");
		exit(1);
	}

        struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, socketPath, sizeof(addr.sun_path) - 1);
	unlink(socketPath);
	if (bind(serverSocket,(struct sockaddr*) &addr, sizeof(addr)) == -1) {
		perror("Bind failed");
		exit(1);
	}

	if (listen(serverSocket, 1) == -1) {
		close(serverSocket);
		unlink(socketPath);
		perror("Listen failed");
		exit(1);
	}

	int clientSocket  = accept(serverSocket, NULL, NULL);

	if (clientSocket == -1) {
		close(serverSocket);
		unlink(socketPath);
		perror("Accept failed");
		exit(1);
	}

	printf("Accepted client.\n");

        ssize_t bytes_read;
        char buffer[BUFFSIZE];
        while((bytes_read = read(clientSocket, buffer, BUFFSIZE)) > 0) {
            for(int i = 0; i < bytes_read; i++) {
                buffer[i] = toupper((unsigned char)buffer[i]);
            }
            printf("%.*s", (int)bytes_read, buffer);
        }

	if (bytes_read == -1) {
		close(clientSocket);
		close(serverSocket);
		unlink(socketPath);
		perror("Read failed");
		exit(1);
	}

	close(clientSocket);
	close(serverSocket);
	unlink(socketPath);
	exit(0);
}
