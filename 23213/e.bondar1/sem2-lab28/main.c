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
#define HTTP_BUFFER_SIZE (64 * 1024)

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

int add_data_to_buffer(int sock, char *main_buffer, int buffer_size, int *buffer_pos) {
	char temp_buffer[BUFSIZ];

	if (*buffer_pos + sizeof(temp_buffer) >= buffer_size) {
		return 0;
	}

	int bytes = read(sock, temp_buffer, sizeof(temp_buffer) - 1);

	if (bytes <= 0) return bytes;

	memcpy(main_buffer + *buffer_pos, temp_buffer, bytes);
	*buffer_pos += bytes;
	main_buffer[*buffer_pos] = '\0';

	return bytes;
}

void clean_displayed_data(char *buffer, int *buffer_pos, int *display_pos) {
	if (*display_pos > 0) {
		int remaining = *buffer_pos - *display_pos;
		memmove(buffer, buffer + *display_pos, remaining);

		*buffer_pos = remaining;
		*display_pos = 0;

		buffer[*buffer_pos] = '\0';
	}
}

void display_next_page(char *buffer, int *display_pos, int buffer_size) {
	int lines_shown = 0;

	while (*display_pos < buffer_size && lines_shown < LINES_PER_PAGE) {
		if (buffer[*display_pos] == '\r') {
			(*display_pos)++;
			continue;
		}
		if (buffer[*display_pos] == '\n') {
			lines_shown++;
		}
		putchar(buffer[*display_pos]);
		(*display_pos)++;
	}

	if (*display_pos < buffer_size) {
		printf("Press space to scroll down...\n");
	}
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

	char buffer[HTTP_BUFFER_SIZE];
	int buffer_pos = 0;
	int display_pos = 0;

	fd_set read_fds;
	int max_fd = (sock > STDIN_FILENO) ? sock : STDIN_FILENO;
	int running = 1;

	while (running) {
		FD_ZERO(&read_fds);
		FD_SET(sock, &read_fds);
		FD_SET(STDIN_FILENO, &read_fds);

		if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
			perror("select");
			break;
		}

		if (FD_ISSET(sock, &read_fds)) {
			if (add_data_to_buffer(sock, buffer, sizeof(buffer), &buffer_pos) <= 0) {
				running = 0;
			}
		}

		if (FD_ISSET(STDIN_FILENO, &read_fds)) {
			char ch;
			if (read(STDIN_FILENO, &ch, 1) > 0 && ch == ' ') {
				display_next_page(buffer, &display_pos, buffer_pos);

				clean_displayed_data(buffer, &buffer_pos, &display_pos);
			}
		}

	}

	restore_canonical();
	close(sock);
	exit(EXIT_SUCCESS);
}
