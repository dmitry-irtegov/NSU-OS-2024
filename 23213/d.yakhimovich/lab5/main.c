#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 512
#define LINE_CAP 64

typedef struct {
    off_t offset;
    off_t length;
} Line;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s [file]\n", argv[0]);
        return 1;
    }

    int fd = open(argv[argc - 1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    Line *lines = malloc(LINE_CAP * sizeof(Line));
    if (!lines) {
        perror("malloc");
        close(fd);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    int line_count = 0;
    int capacity = LINE_CAP;
    off_t offset = 0;
    ssize_t bytes_read;
    lines[line_count++].offset = 0;

    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') {
                lines[line_count - 1].length =
                    offset + i + 1 - lines[line_count - 1].offset;
                if (line_count >= capacity) {
                    capacity *= 2;
                    lines = realloc(lines, capacity * sizeof(Line));
                    if (!lines) {
                        perror("realloc");
                        close(fd);
                        return 1;
                    }
                }
                lines[line_count++].offset = offset + i + 1;
            }
        }
        offset += bytes_read;
    }
    line_count--;
    if (bytes_read == -1) {
        perror("read");
        free(lines);
        close(fd);
        return 1;
    }

    ssize_t remain;
    int line_number;
    int scan_res;

    while (1) {
        printf("Enter line number (0 for exit): ");
        scan_res = scanf("%d", &line_number);
        if (scan_res == EOF) {
             if (ferror(stdin)) {
                perror("input reading error");
            } else {
                fprintf(stderr, "End of input\n");
            }
            return 1;
        }
        if (scan_res == 0) {
            scanf("%*[^\n]");
            printf("Incorrect input.\n");
            continue;
        }
        if (line_number == 0) {
            break;
        }
        if (line_number > 0 && line_number <= line_count) {
            char buffer[BUFFER_SIZE];
            remain = lines[line_number - 1].length;
            bytes_read = 1;

            lseek(fd, lines[line_number - 1].offset, SEEK_SET);
            while (bytes_read > 0 && remain > 0) {
                if (remain > BUFFER_SIZE) {
                    bytes_read = read(fd, buffer, BUFFER_SIZE);
                } else {
                    bytes_read = read(fd, buffer, remain);
                }
                if (bytes_read == -1) {
                    perror("read");
                    return 1;
                }
                if (bytes_read == 0) {
                    break;
                }
                fwrite(buffer, sizeof(char), bytes_read, stdout);
                remain -= bytes_read;
            }

        } else {
            printf("Line number out of range!\n");
        }
    }

    free(lines);
    close(fd);
    return 0;
}
