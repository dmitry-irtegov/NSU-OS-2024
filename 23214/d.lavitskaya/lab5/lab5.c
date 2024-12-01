#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 256

typedef struct {
    off_t offset;
    size_t length;
} LineInfo;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Correct usage: %s <file>\n", argv[0]);
        return 1;
    }


    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("fd open failed");
        return 1;
    }

    int line_count = 0;
    size_t size = MAX_LINES;
    LineInfo *lines = malloc(size * sizeof(LineInfo));
    if (lines == NULL) {
        perror("lines malloc failed");
        close(fd);
        return 1;
    }
    char buffer[MAX_LINE_LENGTH];
    off_t current_offset = 0;
    ssize_t bytes_read;
    lines[0].offset = 0;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        size_t i;
        for (i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') {
                if(line_count >= MAX_LINES) {
                    size *= 2;
                    lines = realloc(lines, size * sizeof(LineInfo));
                    if (lines == NULL) {
                        perror("realloc failed");
                        free(lines);
                        close(fd);
                        return 1;
                    }
                }
                lines[line_count].length = current_offset + i - lines[line_count].offset;
                lines[++line_count].offset = current_offset + i + 1;

            }
        }
        current_offset += bytes_read;
    }

    if (bytes_read == -1) {
        perror("file read failed");
        free(lines);
        close(fd);
        return 1;
    }

    printf("Line index | Offset | Length\n");
    printf("--------------------------------\n");
    size_t i;
    for (i = 0; i < line_count; i++) {
        printf("%10ld | %6ld | %6zu\n", i + 1, lines[i].offset, lines[i].length);
    }
    printf("--------------------------------\n");

    
    while (1) {
        printf("Enter line number (or 0 to quit): ");

        char input_buffer[100];  

         if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
            perror("Failed to read input");
            return 1;
        }

        if (input_buffer[strlen(input_buffer) - 1] != '\n') {
            fprintf(stderr, "Invalid input format.\n");
            while (getchar() != '\n'); 
            continue; 
        }

        char *endptr;
        int line_number = strtol(input_buffer, &endptr, 10);

        if (endptr == input_buffer || *endptr != '\n') {
            fprintf(stderr, "Invalid input format.\n");
            continue;
        }

        if (line_number < 0 || line_number > line_count) {
            fprintf(stderr, "Invalid line number.\n");
            continue;
        }

        if (line_number == 0) {
            break;
        }

        if (lseek(fd, lines[line_number - 1].offset, SEEK_SET) == -1) {
            perror("lseek failed");
            free(lines);
            close(fd);
            return 1;
        }

        size_t len = lines[line_number - 1].length;
        char *line = malloc(len + 1);
        if (line == NULL) {
            perror("line malloc failed");
            free(lines);
            close(fd);
            return 1;
        }

        if (read(fd, line, len) != len) {
            perror("line read failed");
            free(lines);
            free(line);
            close(fd);
            return 1;
        }

        line[len] = '\0';
        printf("Line %d: %s\n", line_number, line);
        free(line);
    }

    free(lines);
    close(fd);
    exit(0);
}
