#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>

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
	fv->line = (LineEntry*)malloc(sizeof(LineEntry) * 5);
	if (fv->line == NULL) {
		perror("Failed to malloc()");
		exit(EXIT_FAILURE);
	}
}

void file_vector_push(FileVector* fv, off_t offset, off_t length) {
	if (fv->size == fv->capacity) {
		fv->capacity *= 2;
		fv->line = (LineEntry*)realloc(fv->line, sizeof(LineEntry) * fv->capacity);
		if (fv->line == NULL) {
			perror("Failed to realloc()");
			exit(EXIT_FAILURE);
		}
	}

	fv->line[fv->size].line_length = length;
	fv->line[fv->size].line_offset = offset;
	fv->size++;
}

void build_file_table(FileVector* fv, const char* data, off_t file_size, int* line_count) {
	file_vector_init(fv);
	off_t line_start = 0;
	*line_count = 0;

	for (off_t i = 0; i < file_size; i++) {
		if (data[i] == '\n') {
			file_vector_push(fv, line_start, i - line_start);
			line_start = i + 1;
			(*line_count)++;
		}
	}
}

void print_line(const char* data, FileVector* fv, int line_number) {
	off_t line_offset = fv->line[line_number].line_offset;
	off_t line_length = fv->line[line_number].line_length;

	write(STDOUT_FILENO, data + line_offset, line_length);
	printf("\n");
}

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [file]\n", argv[0]);
		exit(EXIT_SUCCESS);
	}

	int fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		perror("Failed to open file");
		exit(EXIT_FAILURE);
	}


	struct stat st;
	if (fstat(fd, &st) == -1) {
		perror("Failed to get file status");
		close(fd);
		exit(EXIT_FAILURE);
	}

	off_t file_size = st.st_size;

	char* data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (data == MAP_FAILED) {
		perror("Failed to mmap");
		close(fd);
		exit(EXIT_FAILURE);
	}

	int line_count;
	FileVector fv;
	build_file_table(&fv, data, file_size, &line_count);
	int line_number;

	if (line_count == 0) {
		fprintf(stderr, "Empty file. Nothing to inspect. Probably you have a file that doesn't end with an empty line.\nExiting.\n");
		munmap(data, file_size);
		close(fd);
		exit(EXIT_SUCCESS);
	}

	while (1) {
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
			print_line(data, &fv, line_number - 1);
		}
	}

	munmap(data, file_size);
	close(fd);

	exit(EXIT_SUCCESS);
}

