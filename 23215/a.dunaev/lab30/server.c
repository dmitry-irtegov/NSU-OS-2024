#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#define SOCKET_PATH "/tmp/unix_socket"

int main() {
    int server_fd, client_fd;
    struct sockaddr_un server_addr;
    char buffer[256];

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    unlink(SOCKET_PATH);

    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
	
   	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
   		perror("Bind failed");
   		close(server_fd);
   		return 1;
   	}
   	if (listen(server_fd, 1) == -1) { // Allow up to 5 pending connections
   	   	perror("Listen failed");
   		close(server_fd);
   	    return 1;
   	}
	client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1){
		printf("Failed to connect client");
		close(client_fd);
	    close(server_fd);
	    unlink(SOCKET_PATH);
		return 1;
    }
    int n;
    while ((n = read(client_fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        for (int i = 0; buffer[i]; i++) {
            buffer[i] = toupper(buffer[i]);
        }
        printf("%s", buffer);
    }

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
