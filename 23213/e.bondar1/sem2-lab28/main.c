#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <termios.h>
#include <sys/select.h>

#define LINES_PER_PAGE 25

static struct termios orig_term;

void set_non_canonical() {
	tcgetattr(STDIN_FILENO, &orig_term);

	struct termios new_term = orig_term;
	new_term.c_lflag &= ~(ICANON | ECHO);
	new_term.c_cc[VMIN] = 1;
	new_term.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
}

void restore_canonical() {
	tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
}

int process_data(int sock) {
    static int lines_printed = 0;
    char buffer[BUFSIZ];
    int bytes_read = read(sock, buffer, sizeof(buffer) - 1);

    if (bytes_read <= 0) return bytes_read;

    buffer[bytes_read] = '\0';

    char *line_start = buffer;
    char *newline_pos;

    while ((newline_pos = strchr(line_start, '\n')) != NULL) {
        *newline_pos = '\0';

        char *cr = strchr(line_start, '\r');
        if (cr) *cr = '\0';

        printf("%s\n", line_start);
        lines_printed++;

        if (lines_printed >= LINES_PER_PAGE) {
            printf("Press space to scroll down...\n");

            char ch;
            do {
                read(STDIN_FILENO, &ch, 1);
            } while (ch != ' ');

            lines_printed = 0;
        }

        line_start = newline_pos + 1;
    }

    if (*line_start) {
        printf("%s", line_start);
    }

    return bytes_read;
}

int send_http_request(int sock, const char *host, const char *path, size_t host_len, size_t path_len) {
	char request[host_len + path_len + 50];
	int len = snprintf(request, sizeof(request), "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host);

	if (write(sock, request, len) < 0) {
		perror("write");
		return -1;
	}
	return 0;
}

int create_socket(const char *host) {
	struct hostent *server;
	struct sockaddr_in server_addr;
	int sock;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		return -1;
	}

	server = gethostbyname(host);
	if (server == NULL) {
		fprintf(stderr, "Error: No such host\n");
		close(sock);
		return -1;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	server_addr.sin_port = htons(80);

	if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("connect");
		close(sock);
		return -1;
	}

	return sock;
}

void parse_url(const char *url, size_t *host_len, size_t *path_len,
		const char **host_start, const char **path_start) {
	const char *protocol_end = strstr(url, "://");
	const char *hostname_begin = protocol_end ? protocol_end + 3 : url;
	const char *path_delimiter = strchr(hostname_begin, '/');

	*host_start = hostname_begin;

	if (path_delimiter) {
		*host_len = path_delimiter - hostname_begin;
		*path_start = path_delimiter;
		*path_len = strlen(path_delimiter);
	} else {
		*host_len = strlen(hostname_begin);
		*path_start = "/";
		*path_len = 1;
	}
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int sock;
	size_t host_len, path_len;
	const char *host_start, *path_start;

	parse_url(argv[1], &host_len, &path_len, &host_start, &path_start);
	char host[host_len + 1];
	char path[path_len + 1];

	strncpy(host, host_start, host_len);
	host[host_len] = '\0';

	strcpy(path, path_start);

	sock = create_socket(host);
	if (sock < 0) {
		exit(EXIT_FAILURE);
	}

	if (send_http_request(sock, host, path, host_len, path_len) != 0) {
		close(sock);
		exit(EXIT_FAILURE);
	}

	set_non_canonical();

	fd_set read_fds;
	int running = 1;

	while (running) {
		FD_ZERO(&read_fds);
		FD_SET(sock, &read_fds);

		if (select(sock + 1, &read_fds, NULL, NULL, NULL) < 0) {
			perror("select");
			break;
		}

		if (FD_ISSET(sock, &read_fds)) {
			if (process_data(sock) <= 0) {
				running = 0;
			}
		}
	}

	restore_canonical();
	close(sock);
	exit(EXIT_SUCCESS);
}
