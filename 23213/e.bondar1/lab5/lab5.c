#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct {
    off_t line_offset;
    size_t line_length;
} LineEntry;

LineEntry* build_file_table(int fd, int* line_count) {
    LineEntry* file_table = NULL;
    ssize_t bytes_read;
    size_t line_start = 0;
    off_t curr_offset = 0;

    char buffer[BUFSIZ];
    *line_count = 0;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') {
                file_table = realloc(file_table, (*line_count + 1) * sizeof(LineEntry)); 
                if (file_table == NULL) {
                    perror("Failed to realloc()");
                    exit(EXIT_FAILURE);
                }

                file_table[*line_count].line_length = curr_offset + i - line_start;
                file_table[*line_count].line_offset = line_start;
                
                line_start = curr_offset + i;
                (*line_count)++;
            }
        }
        curr_offset += bytes_read;
    }

    if (bytes_read == -1) {
        perror("Error reading file");
        exit(EXIT_FAILURE);
    }
    return file_table;
}

void print_line(int fd, LineEntry* file_table, int line_number) {
    if (lseek(fd, file_table[line_number].line_offset, SEEK_SET) == -1) {
        perror("Failed to lseek()");
        exit(EXIT_FAILURE);
    }
    
    char* line_buffer = (char*)malloc(file_table[line_number].line_length + 1);
    if (line_buffer == NULL) {
        perror("Failed to malloc()");
        exit(EXIT_FAILURE);
    }

    if (read(fd, line_buffer, file_table[line_number].line_length + 1) == -1) {
        perror("Failed to read line");
        free(line_buffer);
        exit(EXIT_FAILURE);
    }

    line_buffer[file_table[line_number].line_length] = '\0';
    printf("%s\n", line_buffer);
    free(line_buffer);
}

int main(int argc, char** argv) {
	if(argc < 2) {
		fprintf(stderr, "Usage: %s [file]\n", argv[0]);
        exit(EXIT_SUCCESS);
	}
    
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    int line_count;
    LineEntry* file_table = build_file_table(fd, &line_count);

    int line_number;
    printf("Enter line number from 1 to %d\nPrint 0 to Exit\n", line_count);
    scanf("%d", &line_number);

    while (line_number != 0) {
        if (line_number < 1 || line_number > line_count) {
            fprintf(stderr, "Invalid line number\n");
        } else {
            print_line(fd, file_table, line_number - 1);
        }
        printf("Enter line number from 1 to %d\nPrint 0 to Exit\n", line_count);
        scanf("%d", &line_number);       
    }
    
    free(file_table);
    if (close(fd) == -1) {
        perror("Failed to close");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
