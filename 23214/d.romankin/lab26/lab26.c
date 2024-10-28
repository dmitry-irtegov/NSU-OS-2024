#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


int main() {
	FILE* pipeIn;
	char text[BUFSIZ];
	if ((pipeIn = popen("cat test.txt", "r")) == NULL) {
		perror("popen error");
		exit(EXIT_FAILURE);
	}
	while (fgets(text, BUFSIZ, pipeIn) != NULL) {
		if (ferror(pipeIn) != 0) {
			perror("pipeIn error");
			exit(EXIT_FAILURE);
		}
		for (int i = 0; text[i] != '\0'; i++) {
			text[i] = toupper(text[i]);
		}
		printf("%s", text);
	}
	if (pclose(pipeIn) == -1) {
		perror("pclose error");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
