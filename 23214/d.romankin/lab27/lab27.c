#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char** argv) {
	if (argc != 2) {
		fprintf(stderr, "invalid arguments\n");
		exit(EXIT_FAILURE);
	}
	FILE* in;
	FILE* out;
	in = fopen(argv[1], "r");
	if (in == NULL) {
		perror("fopen error");
		exit(EXIT_FAILURE);
	}
	
	out = popen("wc -l", "w");
	if (out == NULL) {
		perror("popen error");
		exit(EXIT_FAILURE);
	}
	char* buffer = NULL;
	size_t lenstr = 0;
	int len;
	int flag = 1;
	while (getline(&buffer, &lenstr, in) != -1) {
		
		len = strlen(buffer) - 1;
		
		if (len > -1 && buffer[len] == '\n') {
			flag = 1;
		}
		else {
			flag = 0;
		}
		if (buffer[0] == '\n') {
			if (fputc('\n', out) == EOF) {
				fprintf(stderr, "fputc error\n");
				free(buffer);
				pclose(out);
				fclose(in);
				exit(EXIT_FAILURE);
			}
		}
	}
	if (ferror(in) != 0) {
		fprintf(stderr, "fgets returned error\n");
		exit(EXIT_FAILURE);
	}

	if (flag) {
		if (fputc('\n', out) == EOF) {
			fprintf(stderr, "fputc error\n");
			free(buffer);
			pclose(out);
			fclose(in);
			exit(EXIT_FAILURE);
		}
	}

	pclose(out);
	fclose(in);
	exit(EXIT_SUCCESS);

}
