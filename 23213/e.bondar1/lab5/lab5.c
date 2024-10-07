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

void file_vector_init(FileVector* fv) {
	fv->capacity = 5;
	fv->size = 0;
	fv->line = (LineEntry*)malloc(sizeof(LineEntry)*5);
	if (fv->line == NULL) {
		perror("Failed to malloc()");
		exit(EXIT_FAILURE);
	}
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

void build_file_table(FileVector* fv, int fd, int* line_count) {
	file_vector_init(fv);
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
}

void print_line(int fd, FileVector* fv, int line_number) {
	if (lseek(fd, fv->line[line_number].line_offset, SEEK_SET) == -1) {
		perror("Failed to lseek()");
		exit(EXIT_FAILURE);
	}

	char buffer[BUFSIZ];
	ssize_t bytes_read;
	off_t line_length = fv->line[line_number].line_length;

	while(line_length > 0) {
		off_t chunk_length = (line_length > (off_t)sizeof(buffer)) ? (off_t)sizeof(buffer) : line_length;

		bytes_read = read(fd, buffer, chunk_length);
		if (bytes_read == -1) {
			perror("Failed to read file");
			exit(EXIT_FAILURE);
		}

		if (bytes_read == 0) {
			break;
		}

		write(STDOUT_FILENO, buffer, bytes_read);
		line_length -= bytes_read;

	}
	printf("\n");
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
	FileVector fv;
	build_file_table(&fv, fd, &line_count);
	int line_number;

	if (line_count == 0) {
		fprintf(stderr, "Empty file. Nothing to inspect. Probably you have file that dont ends with empty line.\nExiting.\n");
		free(fv.line);
		exit(EXIT_SUCCESS);
	}

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
			print_line(fd, &fv, line_number - 1);
		}

	}

	if (close(fd) == -1) {
		perror("Failed to close");
		exit(EXIT_FAILURE);
	}

	free(fv.line);
	exit(EXIT_SUCCESS);
}

