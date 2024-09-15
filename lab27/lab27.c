#define BUFFER_SIZE 8192

#include <stdio.h>

int main(int argc, char** argv) {
	char line[BUFFER_SIZE];
	FILE* inputFile;
	FILE* outputFile;
	int counter = 0;

	inputFile = fopen(argv[1], "r");
	if (inputFile == NULL) {
		fprintf(stderr, "Failed to open file\n");
		return 0;
	}

	outputFile = popen("wc -l", "w");

	while (fgets(line, BUFFER_SIZE, inputFile) != NULL) {
		if (line[0] == '\n' || line[0] == '\r') {
			fputs(line, outputFile);
		}
	}
	fclose(inputFile);
	pclose(outputFile);

	return 0;
}