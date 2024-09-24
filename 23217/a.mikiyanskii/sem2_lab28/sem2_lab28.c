#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <termios.h>
#include <sys/select.h>
#include <iconv.h>
#include <signal.h>

#define LINES_PER_SCREEN 25
#define INITIAL_BUFFER_SIZE 65536
#define RECEIVE_CHUNK_SIZE 4096
#define HTTP_STANDARD_PORT 80
#define REQUEST_BUFFER_SIZE 2048

typedef struct {
    char* text_data;
    size_t current_size;
    size_t max_capacity;
    size_t read_position;
} PageStorage;

typedef struct {
    char domain[256];
    char resource_path[1024];
    int port_number;
} ParsedUrl;

typedef enum {
    HEADER_PROCESSING,
    CONTENT_DISPLAY,
    WAITING_FOR_INPUT
} ReaderState;

int total_lines_since_last_pause = 0;
char detected_charset[64] = "UTF-8";

bool configure_terminal_io(bool enable_raw_mode) {
    static struct termios original_term_settings;
    struct termios new_settings;

    if (tcgetattr(STDIN_FILENO, &original_term_settings) == -1) return false;

    if (enable_raw_mode) {
        new_settings = original_term_settings;
        new_settings.c_lflag &= ~(ICANON | ECHO);
        new_settings.c_cc[VMIN] = 1;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &new_settings) == -1) return false;
    } else {
        if (tcsetattr(STDIN_FILENO, TCSANOW, &original_term_settings) == -1) return false;
    }
    return true;
}

void initialize_page_storage(PageStorage* storage) {
    storage->text_data = malloc(INITIAL_BUFFER_SIZE);
    storage->max_capacity = storage->text_data ? INITIAL_BUFFER_SIZE : 0;
    storage->current_size = 0;
    storage->read_position = 0;
}

bool append_to_page_storage(PageStorage* storage, const char* new_data, size_t data_length) {
    while (storage->current_size + data_length + 1 > storage->max_capacity) {
        storage->max_capacity *= 2;
        char* resized_buffer = realloc(storage->text_data, storage->max_capacity);
        if (!resized_buffer) return false;
        storage->text_data = resized_buffer;
    }
    memcpy(storage->text_data + storage->current_size, new_data, data_length);
    storage->current_size += data_length;
    storage->text_data[storage->current_size] = '\0';
    return true;
}

void cleanup_page_storage(PageStorage* storage) {
    free(storage->text_data);
    storage->text_data = NULL;
}

char* convert_encoding(const char* input, size_t length, const char* from_charset) {
    iconv_t cd = iconv_open("UTF-8", from_charset);
    if (cd == (iconv_t)-1) return NULL;

    size_t inbytesleft = length;
    size_t outbytesleft = length * 4;
    char* output = malloc(outbytesleft + 1);
    if (!output) return NULL;

    char* inbuf = (char*)input;
    char* outbuf = output;
    char* outbuf_start = output;

    if (iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft) == (size_t)-1) {
        free(outbuf_start);
        iconv_close(cd);
        return NULL;
    }

    *outbuf = '\0';
    iconv_close(cd);
    return outbuf_start;
}

void extract_url_components(const char* input_url, ParsedUrl* url_info) {
    url_info->port_number = HTTP_STANDARD_PORT;
    const char* url_start = input_url;
    if (strncmp(input_url, "http://", 7) == 0) url_start = input_url + 7;

    const char* port_indicator = strchr(url_start, ':');
    const char* path_start = strchr(url_start, '/');
    const char* domain_end = path_start ? path_start : url_start + strlen(url_start);

    if (port_indicator && port_indicator < domain_end) {
        strncpy(url_info->domain, url_start, port_indicator - url_start);
        url_info->domain[port_indicator - url_start] = '\0';
        url_info->port_number = atoi(port_indicator + 1);
    } else {
        strncpy(url_info->domain, url_start, domain_end - url_start);
        url_info->domain[domain_end - url_start] = '\0';
    }

    strcpy(url_info->resource_path, path_start ? path_start : "/");
}

int create_and_connect_socket(const ParsedUrl* url_info) {
    char port_string[16];
    snprintf(port_string, sizeof(port_string), "%d", url_info->port_number);

    struct addrinfo hints = {0}, *res;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int sock = -1;
    if (getaddrinfo(url_info->domain, port_string, &hints, &res) != 0) return -1;

    for (struct addrinfo* p = res; p; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == -1) continue;
        if (connect(sock, p->ai_addr, p->ai_addrlen) == 0) break;
        close(sock);
        sock = -1;
    }

    freeaddrinfo(res);
    return sock;
}

void display_content(PageStorage* page, bool* needs_pause) {
    size_t current_pos = page->read_position;
    size_t bytes_printed = 0;
    *needs_pause = false;

    while (current_pos + bytes_printed < page->current_size && total_lines_since_last_pause < LINES_PER_SCREEN) {
        char* line_end = memchr(page->text_data + current_pos + bytes_printed, '\n', page->current_size - current_pos - bytes_printed);

        size_t line_len;
        if (line_end) {
            line_len = line_end - (page->text_data + current_pos + bytes_printed) + 1;
        } else {
            line_len = page->current_size - current_pos - bytes_printed;
        }

        char* line_start = page->text_data + current_pos + bytes_printed;
        char* utf8_line = (strcasecmp(detected_charset, "UTF-8") != 0)
                          ? convert_encoding(line_start, line_len, detected_charset)
                          : strndup(line_start, line_len);

        if (utf8_line) {
            write(STDOUT_FILENO, utf8_line, strlen(utf8_line));
            free(utf8_line);
        }

        bytes_printed += line_len;

        if (line_end) {
            total_lines_since_last_pause++;
        } else {
            total_lines_since_last_pause++;
            break;
        }
    }

    page->read_position += bytes_printed;

    if (total_lines_since_last_pause >= LINES_PER_SCREEN && page->read_position < page->current_size) {
        printf("\n-- MORE -- (press SPACE)");
        fflush(stdout);
        *needs_pause = true;
    }
}

void restore_terminal() {
    configure_terminal_io(false);
}
void handler(int signal_number) {
    restore_terminal();
    exit(EXIT_FAILURE);
}

void signal_handler() {
    struct sigaction signal_action;
    signal_action.sa_handler = handler;
    sigemptyset(&signal_action.sa_mask);
    signal_action.sa_flags = 0;

    sigaction(SIGINT, &signal_action, NULL);
    sigaction(SIGTERM, &signal_action, NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    signal_handler();
    atexit(restore_terminal);

    ParsedUrl url_info = {0};
    extract_url_components(argv[1], &url_info);

    int sock = create_and_connect_socket(&url_info);
    if (sock == -1) {
        fprintf(stderr, "Connection failed\n");
        return EXIT_FAILURE;
    }

    char request[REQUEST_BUFFER_SIZE];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n",
             url_info.resource_path, url_info.domain);
    send(sock, request, strlen(request), 0);

    if (!configure_terminal_io(true)) {
        close(sock);
        return EXIT_FAILURE;
    }

    PageStorage page = {0};
    initialize_page_storage(&page);
    bool connected = true, pause_needed = false;
    ReaderState state = HEADER_PROCESSING;
    fd_set fds;

    while (1) {
        FD_ZERO(&fds);
        if (connected) FD_SET(sock, &fds);
        FD_SET(STDIN_FILENO, &fds);


        if (select(sock + 1, &fds, NULL, NULL, NULL) == -1) break;

        if (connected && FD_ISSET(sock, &fds)) {
            char buffer[RECEIVE_CHUNK_SIZE];
            int received = recv(sock, buffer, sizeof(buffer), 0);
            if (received <= 0) {
                connected = false;
                shutdown(sock, SHUT_RD);
            } else {
                if (!append_to_page_storage(&page, buffer, received)) break;
            }
        }


        if (state == HEADER_PROCESSING) {
            char* header_end = strstr(page.text_data, "\r\n\r\n");
            if (header_end) {
                *header_end = '\0';
                char* ct = strcasestr(page.text_data, "Content-Type:");
                if (ct) {
                    char* cs = strcasestr(ct, "charset=");
                    if (cs) {
                        sscanf(cs + 8, "%63s", detected_charset);
                        char* e = strchr(detected_charset, ';'); if (e) *e = '\0';
                        e = strchr(detected_charset, '\r'); if (e) *e = '\0';
                    }
                }
                *header_end = '\r';
                page.read_position = header_end - page.text_data + 4;
                state = CONTENT_DISPLAY;
            }
        }

        if (state == CONTENT_DISPLAY && !pause_needed) {
            display_content(&page, &pause_needed);
            if (pause_needed) state = WAITING_FOR_INPUT;
        }

        if (FD_ISSET(STDIN_FILENO, &fds) && state == WAITING_FOR_INPUT) {
            char ch;
            read(STDIN_FILENO, &ch, 1);
            if (ch == ' ') {
                printf("\n");
                fflush(stdout);
                pause_needed = false;
                total_lines_since_last_pause = 0;

                state = CONTENT_DISPLAY;

                if (page.read_position < page.current_size) {
                    display_content(&page, &pause_needed);
                    if (pause_needed)
                        state = WAITING_FOR_INPUT;
                }
            }
        }


        if (!connected && page.read_position >= page.current_size) break;
    }

    close(sock);
    cleanup_page_storage(&page);
    return EXIT_SUCCESS;
}