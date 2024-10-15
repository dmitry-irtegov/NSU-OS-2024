#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFF_SIZE 1024
#define LINE_MAX 512

int main() {
    const char *filename = "stih.txt";
    int fd = open(filename, O_RDONLY);

    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }

    off_t offsets[BUFF_SIZE];
    size_t lengths[BUFF_SIZE];
    int line_number = 0;
    char buffer[BUFF_SIZE];
    ssize_t bytes_read;
    off_t current_offset = 0;

    offsets[0] = 0;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) { 
            if (buffer[i] == '\n') {
		if (line_number + 1 == LINE_MAX) {
		    fprintf(stderr, "Maximum number of lines has been exceeded.");
                    exit(2);
		}
                lengths[line_number] = current_offset + i - offsets[line_number];
                offsets[++line_number] = current_offset + i + 1;
            }
        }
        current_offset += bytes_read;
    }

    if (bytes_read == -1) {
        perror("Error reading file");
        close(fd);
        exit(3);
    }

    while (1) {
        printf("Enter line number (or 0 to quit): ");
        int input_line;

        int res = scanf("%d", &input_line);

        if (res == EOF) {
            printf("Reached end of file. Quit.\n");
            break;
        }

        if (res == 0) {
	    scanf("%*[^\n]");
            fprintf(stderr, "Invalid input.\n");
            continue;
        }

        if (input_line < 0 || input_line > line_number) {
            fprintf(stderr, "Invalid line number.\n");
            continue;
        }

        if (input_line == 0) {
            break;
        }

        off_t offset = offsets[input_line - 1];
        size_t length = lengths[input_line - 1];

        if (lseek(fd, offset, SEEK_SET) == -1) {
            perror("Error seeking file");
            close(fd);
            exit(4);
        }

        char *line = malloc(length + 1);
        if (line == NULL) {
            fprintf(stderr, "Memory allocation error.\n");
            close(fd);
            exit(5);
        }

        if (read(fd, line, length) != length) {
            perror("Error reading line");
            free(line);
            close(fd);
            exit(6);
        }

        line[length] = '\0';
        printf("Line %d: %s\n", input_line, line);
        free(line);
    }

    close(fd);
    exit(0);
}
