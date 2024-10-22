#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#define LINES 100

int main(int argc, char* argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Missing text file\n");
        return 1;
    }

    int file_descriptor = open(argv[1], O_RDONLY);
    if(file_descriptor == -1)
    {
        perror("File opening fail");
        return 1;
    }

    size_t current_line = 0, symbol_counter = 0;
    off_t *displacements = NULL;
    off_t *lines_lens = NULL;
    char c;
    size_t startedLinesNumber = LINES;
    
    displacements = (off_t*)malloc(startedLinesNumber * sizeof(off_t));
    if (displacements == NULL) {
        perror("Memory allocation error");
        return 1;
    }

    lines_lens = (off_t*)malloc(startedLinesNumber * sizeof(int));
    if (lines_lens == NULL) {
        perror("Memory allocation error");
        return 1;
    }
    
    char buf[BUFSIZ];
    ssize_t bytes_read = 1;
    displacements[0] = 0;
    while (bytes_read > 0) {
        bytes_read = read(file_descriptor, buf, sizeof(buf) - 1);
        if(bytes_read == -1)
        {
            perror("Reading error");
            return 1;
        }
        for(ssize_t i = 0; i<bytes_read; i++){
            c = buf[i];
            if (c == '\n') {
                lines_lens[current_line] = symbol_counter + 1;
                current_line++;
                symbol_counter = 0;
            
                if (current_line >= startedLinesNumber) {
                    startedLinesNumber *= 2;
                    displacements = (off_t *)realloc(displacements, startedLinesNumber * sizeof(off_t));
                    if (displacements == NULL) {
                        perror("Memory reallocation error");
                        return 1;
                    }
                    lines_lens = (off_t*)realloc(lines_lens, startedLinesNumber * sizeof(off_t));
                    if(lines_lens == NULL) {
                        perror("Mempry reallocation error");
                        return 1;
                    }
                }
                displacements[current_line] = displacements[current_line - 1] + lines_lens[current_line - 1];
            } else {
                symbol_counter++;
            }
        }       
    }

    size_t lineNum = 0;

    while (1) {
        printf("Enter the string number from 1 to %zu (0 to exit): ", current_line);
        int res = scanf("%zu", &lineNum);
    
        if (res == EOF) {
            if(ferror(stdin)) {
                perror("Reading from stdin error");
            }
            printf("\nEOF detected. Program closing...\n");
            break;
        }

        if (res != 1) {
            printf("Incorrect enter. Try again.\n");
            scanf("%*s");
            continue;
        }

        if (lineNum == 0) {
            printf("Program closing...\n");
            break;
        }

        if (lineNum < 1 || lineNum > current_line) {
            fprintf(stderr, "Incorrect string number. Try again. \n");
            continue;
        }

        if (lseek(file_descriptor, displacements[lineNum - 1], SEEK_SET) == -1) {
            perror("Lseek error");
            return 1;
        }

        char line[BUFSIZ];
        off_t len = lines_lens[lineNum - 1];
        ssize_t was_readen = 1;
        while (was_readen != 0 && len > 0)
        {
            if(len < (off_t) sizeof(line)) {
                was_readen = read(file_descriptor, line, len);
            } else {
                was_readen = read(file_descriptor, line, sizeof(line) - 1);
            }
            
            if (was_readen == -1) {
                perror("Reading error");
                return 1;
            }

            len -= was_readen;

            if(write(STDOUT_FILENO, line, was_readen) == -1) {
                perror("Writing error");
                return 1;
            }                        
        }
    }

    free(displacements);
    free(lines_lens);
    close(file_descriptor);
    
    return 0;
}

