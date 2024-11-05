#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#define BUFSIZE 1024
#define COMMAND_SIZE 4

void processReader(FILE *fp) {
    char buffer[BUFSIZE];
    while (fgets(buffer, BUFSIZE, fp) != NULL) {
        for (size_t i = 0; buffer[i] != '\0'; i++) {
            putchar(toupper(buffer[i]));
        }
    }
    putchar('\n');
}

int main() {
    FILE *fp;
    char command[COMMAND_SIZE];

    snprintf(command, COMMAND_SIZE, "cat");

    fp = popen(command, "r");

    if (fp == NULL) {
        perror("popen fail");
        exit(EXIT_FAILURE);
    }

    processReader(fp);

    if (pclose(fp) == -1) {
        perror("pclose fail");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}


