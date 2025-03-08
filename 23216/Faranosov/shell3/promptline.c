#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "shell.h"

void clearLine(char* line) {
	for (int i = 0; i < 1024; i++) line[i] = '\0';
}

int promptline(char* line, int sizeline) {
	int n = 0, gettedcnt = 0;
	clearLine(line);
	while (1) {
		gettedcnt = read(0, (line + n), sizeline - n);

		if (gettedcnt == -1) {
			perror("read error");
			return -1;
		}
		n += gettedcnt;

		*(line + n) = '\0';

		/*
		*  check to see if command line extends onto
		*  next line.  If so, append next line to command line
		*/

		if (*(line + n - 2) == '\\' && *(line + n - 1) == '\n') {
			*(line + n) = ' ';
			*(line + n - 1) = ' ';
			*(line + n - 2) = ' ';
			continue; /*read next line*/
		}
		else break;
	}

	return 0;
}