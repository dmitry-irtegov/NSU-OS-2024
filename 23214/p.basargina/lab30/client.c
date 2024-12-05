#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define BUFFSIZE 1024

int main() {
        char* socketPath = "./socket_file";

	int clientSocket;
	clientSocket = socket(AF_UNIX, SOCK_STREAM, 0);

	if (clientSocket == -1) {
		perror("Socket creation failed");
		exit(1);
	}

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, socketPath, sizeof(addr.sun_path) - 1);

	if (connect(clientSocket,(struct sockaddr*)&addr, sizeof(addr)) == -1) {
		close(clientSocket);
		perror("Connect failed");
		exit(1);
	}

	printf("Connected to server socket.\n");

        ssize_t bytes_read;
        char buffer[BUFFSIZE];
        int fd = fileno(stdin);
        while ((bytes_read = read(fd, buffer, BUFFSIZE)) > 0) {
            if (write(clientSocket, buffer, bytes_read) == -1) {
                close(clientSocket);
                perror("Write failed");
                exit(1);
            }
        }

        if (bytes_read == -1) {
            close(clientSocket);
            perror("Read failed");
            exit(1);
        }

	close(clientSocket);
	exit(0);
}
