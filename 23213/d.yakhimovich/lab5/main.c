#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 512
#define LINE_CAP 64

typedef struct {
    off_t offset;
    size_t length;
} Line;

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3 || (argc == 3 && argv[1][0] != '-')) {
        fprintf(stderr, "usage: %s [-d] [file]\n", argv[0]);
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
    size_t line_count = 0;
    size_t capacity = LINE_CAP;
    off_t offset = 0;
    ssize_t bytes_read;
    lines[line_count++].offset = 0;

    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') {
                lines[line_count - 1].length = offset + i + 1 - lines[line_count - 1].offset;
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

    if (argc == 3 && (strcmp(argv[1], "-d") == 0 || strcmp(argv[2], "-d") == 0)) {
        for (size_t i = 0; i < line_count; i++) {
            printf("Line %lu: offset = %ld, len = %zu\n", i + 1, lines[i].offset, lines[i].length);
        }
    }

    size_t line_number;
    size_t remain;
    int scan_res;
    char line_end;

    while (1) {
        printf("Enter line number (0 for exit): ");
        scan_res = scanf("%ld", &line_number);
        scanf("%c", &line_end);
        if (scan_res != 1 || line_end != '\n') {
            printf("Invalid input!\n");
            scanf("%*[^\n]");
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
                if (remain >= sizeof(buffer)) {
                    bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
                } else {
                    bytes_read = read(fd, buffer, remain);
                }
                if (bytes_read == -1) {
                    perror("read");
                    return 1;
                }
                remain -= bytes_read;
                buffer[bytes_read] = '\0';
                printf("%s", buffer);
            }
        } else {
            printf("Line number out of range!\n");
        }
    }

    free(lines);
    if (close(fd) != 0) {
        perror("close");
        return 1;
    }
    return 0;
}
