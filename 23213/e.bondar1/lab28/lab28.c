#include <stdio.h>
#include <libgen.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
	srand(time(NULL));

	FILE* fd[2];
	if (p2open("sort -n", fd) == -1) {
		fprintf(stderr, "p2open() failed\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < 100; i++) {
		fprintf(fd[0], "%d\n", rand() % 100);
	}

	if (fclose(fd[0]) == EOF) {
		perror("fclose() failed");
		exit(EXIT_FAILURE);
	}

	int number;
	for (int i = 1; i < 101; i++) {
		if (fscanf(fd[1], "%d\n", &number) == EOF) {
			if (feof(fd[1])) {
				fprintf(stderr, "fscanf() returned unexpected EOF from pipe\n");
			} else if (ferror(fd[1])) {
				perror("failed to read from pipe");
			}
			exit(EXIT_FAILURE); 
		}

		printf("%2d ", number);
		if (i % 10 == 0) {
			printf("\n");
		}
	}	

	if (pclose(fd[1]) == -1) {
		perror("pclose failed");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
