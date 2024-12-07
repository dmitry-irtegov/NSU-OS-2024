#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>

#define MAX_FILES 128
#define TIME_OUT_SEC 5
#define TIME_OUT_USEC 0

typedef struct {
	int fd;
	int is_eof;
} FileDescriptor;

int open_files(char** filenames, int num_files, FileDescriptor* file_array, fd_set* active_fds, int* max_fd) {
	int files_open = 0;
	FD_ZERO(active_fds);
	*max_fd = -1;

	for (int i = 0; i < num_files; i++) {
		int fd = open(filenames[i], O_RDONLY);
		if (fd == -1) {
			perror("Error opening file");
			exit(EXIT_FAILURE);
		}

		file_array[files_open].fd = fd;
		file_array[files_open].is_eof = 0;
		files_open++;

		FD_SET(fd, active_fds);
		if (fd > *max_fd) {
			*max_fd = fd;
		}
	}

	return files_open;
}

int handle_file_input(FileDescriptor *file, char *filename) {
	char buffer[BUFSIZ];
	ssize_t bytes_read = read(file->fd, &buffer, sizeof(buffer));

	if (bytes_read > 0) {
		buffer[bytes_read] = '\0';
		printf("Read from file %s: %s\n", filename, buffer);
		return 1; 
	} else if (bytes_read == 0) {
		close(file->fd);
		file->is_eof = 1;
		printf("File %s is reached EOF\n", filename);
		return 0;
	} else {
		perror("read error");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <file1> [file2] ...\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (argc - 1 > MAX_FILES) {
		fprintf(stderr, "Too many files. Maximum is %d\n", MAX_FILES);
		exit(EXIT_FAILURE);
	}

	FileDescriptor file_array[MAX_FILES] = {0};
	fd_set active_fds;
	int max_fd;

	int files_open = open_files(argv + 1, argc - 1, file_array, &active_fds, &max_fd);
	int active_files = files_open;

	while (active_files > 0) {
		fd_set read_fds;
		memcpy(&read_fds, &active_fds, sizeof(fd_set));

		struct timeval timeout;
		timeout.tv_sec = TIME_OUT_SEC;
		timeout.tv_usec = TIME_OUT_USEC;

		int ready = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

		if (ready == -1) {
			perror("select failed");
			exit(EXIT_FAILURE);
		}

		if (ready == 0) {
			printf("Timeout: no input within %d seconds\n", TIME_OUT_SEC);
			continue;
		}

		for (int i = 0; i < files_open; i++) {
			if (file_array[i].is_eof) continue;

			if (FD_ISSET(file_array[i].fd, &read_fds)) {
				if ((handle_file_input(&file_array[i], (argv+1)[i])) == 0) {
					active_files--;
					FD_CLR(file_array[i].fd, &active_fds);
				}
			}
		}
	}

	exit(EXIT_SUCCESS);
}
