#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include "network.h"
#include "cache.h"

#define BUFFER_SIZE 4096
#define HOST "127.0.0.1:6000"
#define STAY_CONNECTION_IN_WRITING_FROM_BUFFER 1
#define CLIENT_TIMEOUT 10

int sockfd;
int server_socket;

client* client_head = NULL;

time_t last_log;

int lg = 1;

void logs() {
	time_t lst = time(NULL);
	if (lst - last_log < 10) {
		return;
	}
	last_log = lst;
	client* cur = client_head;
	//if (!cur && !lg) return;
	printf("\nClients log:\n");
	if (!cur) {
		printf(">No clients");
		lg = 0;
	}
	else while (cur) {
		printf(">%s\n", cur->host ? cur->host : "(no host)");
		cur = cur->next;
		lg = 1;
	}
	printf("\n");
	cache* cc = cache_head;
	printf("\nCache log:\n");
	if (!cc) {
		printf("==============================================\nNo cache\n==============================================\n");
	}
	else while (cc) {
		printf("==============================================\n%s==============================================\n", cc->request);
		/*data* d = cc->dat;
		while (d) {

			printf("----------------------------------\n%s\n----------------------------------\n", d->data);
			d = d->next;
		}*/
		cc = cc->next;
	}
	printf("\n");
}

void signal_handler(int sig) {
	close(server_socket);
	client* cur = client_head;
	while (cur) {
		client* next = cur->next;
		close(cur->cli_fd);
		if (cur->inet_fd > 0) close(cur->inet_fd);
		free(cur->host);
		free(cur);
		cur = next;
	}
	cache* cache_cur = cache_head;
	while (cache_cur) {
		cache* next = cache_cur->next;
		remove_from_cache(cache_cur);
		cache_cur = next;
	}
	exit(0);
}

void error(const char* msg) {
	perror(msg);
	close(server_socket);
	client* cur = client_head;
	while (cur) {
		client* next = cur->next;
		close(cur->cli_fd);
		if (cur->inet_fd > 0) close(cur->inet_fd);
		free(cur->host);
		free(cur);
		cur = next;
	}
	cache* cache_cur = cache_head;
	while (cache_cur) {
		cache* next = cache_cur->next;
		remove_from_cache(cache_cur);
		cache_cur = next;
	}
	exit(1);
}

int add(int cl_fd) {
	client* a = malloc(sizeof(client));
	if (!a) return -1;

	a->cli_fd = cl_fd;
	set_nonblocking(cl_fd);
	a->next = NULL;
	a->tot = 0;
	a->len = 0;
	a->writing = 0;
	a->header_len = 0;
	a->headers_len = 0;
	a->writing_to_client = 0;
	a->writing_to_client_total = 0;
	a->using_cache = 0;
	a->collect_headers = 0;
	a->headers_collectors = (char*)malloc(4096);
	a->host = (char*)malloc(BUFFER_SIZE);
	if (!a->host) {
		free(a);
		return -1;
	}
	a->host[0] = '\0';
	a->inet_fd = 0;
	a->cur_cache = NULL;
	a->last_activity = time(NULL);

	if (client_head == NULL) {
		client_head = a;
	}
	else {
		client* t = client_head;
		while (t->next) t = t->next;
		t->next = a;
	}



	return 0;
}

int minn(int a, int b) {
	return a < b ? a : b;
}

client* clear_connection(client* cur) {
	client* next = cur->next;
	close(cur->cli_fd);
	if (cur->inet_fd > 0) close(cur->inet_fd);
	client** p = &client_head;
	while (*p && *p != cur) p = &(*p)->next;
	if (*p) *p = next;
	free(cur->host);
	free(cur);
	return next;
}

int main() {
	char buffer[BUFFER_SIZE];
	char hst[1024];
	last_log = time(NULL);

	int client_socket;
	con_to_cli(&server_socket);
	if (listen(server_socket, 5) < 0) {
		error("Îøèáêà listen");
	}

	signal(SIGINT, signal_handler);

	client_socket = accept(server_socket, NULL, NULL);
	//printf("hello\n");

	add(client_socket);

	printf("\nStarting server\n");

	while (1) {
		struct timeval tv;
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		client* cur = client_head;
		fd_set read_fds;
		fd_set write_fds;
		FD_ZERO(&read_fds);
		FD_ZERO(&write_fds);
		FD_SET(server_socket, &read_fds);

		time_t now = time(NULL);
		cur = client_head;
		while (cur) {
			if (now - cur->last_activity > CLIENT_TIMEOUT) {
				printf("\tClient %d timed out\n", cur->cli_fd);
				cur = clear_connection(cur);
				continue;
			}
			cur = cur->next;
		}
		cur = client_head;

		int max_fd = server_socket;
		while (cur) {
			FD_SET(cur->cli_fd, &read_fds);
			FD_SET(cur->cli_fd, &write_fds);
			if (cur->inet_fd > 0)
				FD_SET(cur->inet_fd, &read_fds);
			if (cur->cli_fd > max_fd) max_fd = cur->cli_fd;
			if (cur->inet_fd > max_fd) max_fd = cur->inet_fd;
			cur = cur->next;
		}

		int activity = select(max_fd + 1, &read_fds, &write_fds, NULL, &tv);
		logs();
		if (activity < 0) {
			perror("select");
			exit(1);
		}
		if (activity == 0) {
			check_live_time_cache();
			continue;
		}

		//Get new connection
		if (FD_ISSET(server_socket, &read_fds)) {
			//printf("here\n");
			int client_socket = accept(server_socket, NULL, NULL);
			if (client_socket < 0) {
				perror("accept");
				continue;
			}
			printf("\tNew client %d\n", client_socket);
			add(client_socket);
		}

		cur = client_head;
		while (cur) {
			//printf("%d\n", 1);
			if (FD_ISSET(cur->cli_fd, &read_fds)) {

				int r = read(cur->cli_fd, buffer, BUFFER_SIZE - 1);

				if (r <= 0) {
					printf("\tClient %d disconnected\n", cur->cli_fd);
					cur = clear_connection(cur);
					continue;
				}

				buffer[r] = '\0';
				cur->last_activity = time(NULL);

				if (!cur->writing) {
					cur->writing = 1;
					cur->tot = 0;
					cur->len = 0;
					cur->header_len = 0;
					cur->headers_len = 0;
					cur->writing_to_client = 0;
					cur->writing_to_client_total = 0;
					cur->caching = 1;
					//Find cache
					cache* pot_cache = find_cache(buffer);
					if (pot_cache) {
						printf("---------Used prepared cache\n");
						cur->cur_cache = pot_cache;
						cur->cur_data = pot_cache->dat;
						cur->using_cache = 1;
					}
					else {
						cur->using_cache = 0;
						parse_http_request(buffer, hst);
						cur->cur_cache = add_to_cache(buffer);
						cur->cur_cache->working = 1;
						if (hst[0] != '\0' && (cur->host[0] == '\0' || strcmp(hst, cur->host))) {
							if (cur->inet_fd > 0) {
								close(cur->inet_fd);
								cur->inet_fd = 0;
							}
							con_to_host(&cur->inet_fd, hst);
							strncpy(cur->host, hst, BUFFER_SIZE - 1);
							cur->host[BUFFER_SIZE - 1] = '\0';

							printf("\tNew req %s at %d\n", cur->host, cur->cli_fd);
						}
						if ((r = write(cur->inet_fd, buffer, strlen(buffer))) < 0)
							error("Err: writing in socket");
					}
				}
			}

			int bytes_read = 0;

			//Using buffer with slow client
			if (FD_ISSET(cur->cli_fd, &write_fds) && cur->writing_to_client) {
				cur->writing_to_client += write(cur->cli_fd, cur->buffer + cur->writing_to_client, cur->writing_to_client_total - cur->writing_to_client);
				if (cur->writing_to_client == cur->writing_to_client_total) cur->writing_to_client = 0;

				if (STAY_CONNECTION_IN_WRITING_FROM_BUFFER) { cur->last_activity = time(NULL); }
				printf("+++++++++++++++++Used\n");

			}

			//Using cache
			if (cur->using_cache) {
				//if (cur->cur_data) { printf("ok\n"); }
				//printf("aaaa\n");
				if (FD_ISSET(cur->cli_fd, &write_fds) && !cur->writing_to_client && cur->cur_data && cur->writing) {
					memcpy(cur->buffer, cur->cur_data->data, cur->cur_data->len);
					cur->writing_to_client = write(cur->cli_fd, cur->buffer, cur->cur_data->len);
					cur->writing_to_client_total = cur->cur_data->len;
					if (cur->writing_to_client == cur->cur_data->len) cur->writing_to_client = 0;
					cur->last_activity = time(NULL);
					cur->cur_data = cur->cur_data->next;
					if (!cur->cur_data) {
						cur->writing = 0;
					}
				}
			}
			else {
				if (cur->inet_fd && FD_ISSET(cur->cli_fd, &write_fds) && FD_ISSET(cur->inet_fd, &read_fds) && !cur->writing_to_client && (cur->tot <= cur->len + cur->headers_len || cur->len == -1) &&
					(bytes_read = read(cur->inet_fd, cur->buffer, BUFFER_SIZE - 1)) >= 0) {
					if (!bytes_read) {

						if (cur->cur_cache) cur->cur_cache->working = 0;
						close(cur->inet_fd);
						cur->inet_fd = 0;
						cur->writing = 0;
						printf("Done cache\n");
						cur = cur->next;
						continue;
					}

					//Headers paring
					if (cur->collect_headers != -1) {
						strncpy(cur->headers_collectors + cur->collect_headers, cur->buffer, minn(4096, cur->collect_headers + bytes_read));
						if (strstr(cur->headers_collectors, "\r\n\r\n")) {

							if (!cur->len || cur->cur_cache->live_time == -1 || cur->cur_cache->status_code == -1) {
								int len, live, status;
								parse_headers(cur->buffer, &len, &live, &status);
								if (!cur->len) cur->len = len == -1 ? cur->len : len;
								if (cur->cur_cache->live_time == -1) cur->cur_cache->live_time = live == -1 ? cur->cur_cache->live_time : 60;
								if (cur->cur_cache->status_code == -1) cur->cur_cache->status_code = status == -1 ? cur->cur_cache->status_code : status;
								if ((status != -1 && status / 100 != 2) || len == -1) {
									remove_from_cache(cur->cur_cache);
									cur->cur_cache = NULL;
									printf("Stop caching ");
									if (len == -1) printf(" no len ");
									if (status / 100 != 2) printf(" bad status - %d ", status);
									printf("\n");
								}
							}

							if (!cur->headers_len) {
								char* header_end = strstr(cur->buffer, "\r\n\r\n");
								if (header_end) {
									cur->headers_len = cur->tot + header_end - cur->buffer + 4;

								}
							}
							free(cur->headers_collectors);
							cur->collect_headers = -1;
						}
					}

					//Getting data
					cur->buffer[bytes_read] = '\0';
					cur->writing_to_client = write(cur->cli_fd, cur->buffer, bytes_read);
					cur->writing_to_client_total = bytes_read;
					printf("\tI get %d: %d bytes from %d, and send %d bytes\n", bytes_read, cur->tot + bytes_read, cur->len + cur->headers_len, cur->writing_to_client);
					if (cur->writing_to_client >= bytes_read) cur->writing_to_client = 0;
					cur->tot += bytes_read;
					if (cur->cur_cache && cur->cur_cache->working) {
						add_to_data(cur->cur_cache, cur->buffer, bytes_read);
					}
					if (cur->len != -1 && cur->tot >= cur->len + cur->headers_len) {
						if (cur->cur_cache) cur->cur_cache->working = 0;
						close(cur->inet_fd);
						cur->inet_fd = 0;
						cur->writing = 0;
						printf("Done cache\n");
					}
					cur->last_activity = time(NULL);
				}
				if (bytes_read < 0) {
					error("Err: reading from socket");
				}
			}

			cur = cur->next;
		}
	}

	close(client_socket);
	close(sockfd);
	return 0;
}