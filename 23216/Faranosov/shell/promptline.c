#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "shell.h"

void promptline(char* line, int sizeline) {
	int n = 0;

	while (1) {
		n += read(0, (line + n), sizeline - n);
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
}