#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
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

	if (*buffer_pos >= buffer_size - 1) {
		return 0;
	}

	int space_left = buffer_size - *buffer_pos - 1;
	int read_size = (space_left < sizeof(temp_buffer)) ? space_left : sizeof(temp_buffer) -1;
	int bytes = read(sock, temp_buffer, read_size);

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

void display_next_page(char *buffer, int buffer_size, int *display_pos, int* buffer_pos, int interactive) {
	int lines_shown = 0;
	char last_printed = 0;
	int printed_any = 0;

	while (*display_pos < *buffer_pos && lines_shown < LINES_PER_PAGE) {
		if (buffer[*display_pos] == '\r') {
			(*display_pos)++;
			continue;
		}
		if (buffer[*display_pos] == '\n') {
			lines_shown++;
		}
		putchar(buffer[*display_pos]);
		last_printed = buffer[*display_pos];
		printed_any = 1;
		(*display_pos)++;
	}

	if (interactive && *display_pos < buffer_size) {
		if (printed_any && last_printed != '\n') {
			putchar('\n');
		}
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

	const char *url = argv[1];
	int interactive = isatty(STDOUT_FILENO);

	size_t host_len, path_len;
	const char *host_start, *path_start;

	parse_url(url, &host_len, &path_len, &host_start, &path_start);
	char host[host_len + 1];
	char path[path_len + 1];

	strncpy(host, host_start, host_len);
	host[host_len] = '\0';
	strncpy(path, path_start, path_len);
	path[path_len] = '\0';

	int sock = create_socket(host);
	if (sock < 0) {
		exit(EXIT_FAILURE);
	}

	if (send_http_request(sock, host, path, host_len, path_len) != 0) {
		close(sock);
		exit(EXIT_FAILURE);
	}

	if (interactive) {
		set_non_canonical();

		char buffer[HTTP_BUFFER_SIZE];
		int buffer_pos = 0;
		int display_pos = 0;
		int connection_closed = 0;
		int first_page_shown = 0;
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
					connection_closed = 1;
				}
				if (!first_page_shown) {
					display_next_page(buffer, sizeof(buffer), &display_pos, &buffer_pos, 1);
					first_page_shown = 1;
				}
			}
			if (FD_ISSET(STDIN_FILENO, &read_fds)) {
				char ch;
				if (read(STDIN_FILENO, &ch, 1) > 0 && ch == ' ') {
					display_next_page(buffer, sizeof(buffer), &display_pos, &buffer_pos, 1);
					clean_displayed_data(buffer, &buffer_pos, &display_pos);
				}
			}
			if (connection_closed && display_pos >= buffer_pos) {
				running = 0;
			}
		}

		restore_canonical();
	} else {
		char buffer[HTTP_BUFFER_SIZE];
		int buffer_pos = 0;
		int display_pos = 0;
		int connection_closed = 0;

		while (!connection_closed || display_pos < buffer_pos) {
			if (!connection_closed) {
				if (add_data_to_buffer(sock, buffer, sizeof(buffer), &buffer_pos) <= 0) {
					connection_closed = 1;
				}
			}

			while (display_pos < buffer_pos) {
				display_next_page(buffer, sizeof(buffer), &display_pos, &buffer_pos, 0);
				clean_displayed_data(buffer, &buffer_pos, &display_pos);
			}
		}
	}

	close(sock);
	exit(EXIT_SUCCESS);
}
