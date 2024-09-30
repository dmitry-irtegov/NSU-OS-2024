#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct {
	off_t line_offset;
	off_t line_length;
} LineEntry;

typedef struct {
	LineEntry* line;
	int capacity;
	int size;
} FileVector;

FileVector* file_vector_init(void) {
	FileVector* fv = (FileVector*)malloc(sizeof(FileVector));
	fv->capacity = 5;
	fv->size = 0;
	fv->line = (LineEntry*)malloc(sizeof(LineEntry)*5);
	if (fv->line == NULL) {
		perror("Failed to malloc()");
		exit(EXIT_FAILURE);
	}
	return fv;
}

void file_vector_push(FileVector* fv, off_t offset, off_t length) {
	if (fv->size == fv->capacity) {
		fv->capacity *= 2;
		fv->line = (LineEntry*)realloc(fv->line, sizeof(LineEntry)*fv->capacity);
		if (fv->line == NULL) {
			perror("Failed to realloc()");
			exit(EXIT_FAILURE);
		}
	} 

	fv->line[fv->size].line_length = length;
	fv->line[fv->size].line_offset = offset;
	fv->size++;
}

FileVector* build_file_table(int fd, int* line_count) {
	FileVector* fv = file_vector_init(); 
	ssize_t bytes_read;
	off_t line_start = 0;
	off_t curr_offset = 0;

	char buffer[BUFSIZ];
	*line_count = 0;

	while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
		for (ssize_t i = 0; i < bytes_read; i++) {
			if (buffer[i] == '\n') {
				file_vector_push(fv, line_start, curr_offset + i - line_start);

				line_start = curr_offset + i + 1;
				(*line_count)++;
			}
		}
		curr_offset += bytes_read;
	}

	if (bytes_read == -1) {
		perror("Error reading file");
		exit(EXIT_FAILURE);
	}

	return fv;
}

void print_line(int fd, FileVector* fv, int line_number) {
	if (lseek(fd, fv->line[line_number].line_offset, SEEK_SET) == -1) {
		perror("Failed to lseek()");
		exit(EXIT_FAILURE);
	}

	char buffer[BUFSIZ];
	ssize_t bytes_read;

	while((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
		for (ssize_t i = 0; i < bytes_read; i++) {
			if (buffer[i] == '\n') {
				buffer[i] = '\0';
				printf("%s\n", buffer);
				return;
			}
		}    
		printf("%s", buffer);
	}

	if (bytes_read == -1) {
		perror("Failed to read file");
		exit(EXIT_FAILURE);
	}
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
	FileVector* fv = build_file_table(fd, &line_count);
	int line_number;

	while(1) {
		printf("Enter line number from 1 to %d\nPrint 0 to Exit\n", line_count);

		int scanf_ret = scanf("%d", &line_number);
		if (scanf_ret == EOF) {
			printf("EOF detected, exiting\n");
			break;
		}

		if (scanf_ret != 1) {
			printf("Enter only string number\n");
			char c;
			while ((c = getchar()) != '\n' && c != EOF);
			continue;
		}

		if (line_number == 0) {
			break;
		}

		if (line_number < 1 || line_number > line_count) {
			fprintf(stderr, "Invalid line number\n");
		} else {
			print_line(fd, fv, line_number - 1);
		}

	}

	if (close(fd) == -1) {
		perror("Failed to close");
		exit(EXIT_FAILURE);
	}

	free(fv);
	free(fv->line);
	exit(EXIT_SUCCESS);
}
