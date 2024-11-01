#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <readline/readline.h>

#define minn(a,b) (((a) < (b)) ? (a) : (b))
#define TABLE_SIZE 2048

typedef struct line {
    off_t offset;
    off_t len;
} line;

int is_digit(char* str) {
    if (!str[0] || str[0] == '0' && str[1]) {
        return 0;
    }

    int indx = -1;

    while (++indx < 9 && str[indx]) {
        if (!(str[indx] >= '0' && str[indx] <= '9')) {
            return 0;
        }
    }

    if (str[indx]) {
        return 0;
    }

    return 1;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("File name not found in argv.\n");
        exit(-1);
    }

    const char* filename = argv[1];

    int file_descriptor = open(filename, O_RDONLY);
    if (file_descriptor == -1) {
        printf("Error opening file.\n");
        exit(-1);
    }

    char buffer[BUFSIZ];
    int now_line = 0, pos = 0, len = 0, flag = 1;
    
    ssize_t read_result;
    line table[TABLE_SIZE] = { 0 };

    table[now_line].offset = (off_t)0;

    while (flag && (read_result = read(file_descriptor, buffer, sizeof(buffer))) > 0) {
        for (int i = 0; i < read_result; i++) {
            pos++, len++;

            if (buffer[i] == '\n') {
                table[now_line++].len = len;
                len = 0;

                if (now_line < TABLE_SIZE) {
                    table[now_line].offset = pos;
                }

                else {
                    flag = 0;

                    printf("The last possible table line (%d) is filled. Further reading of the file is stopped.", TABLE_SIZE);
                    break;
                }
            }
        }
    }

    if (read_result == -1) {
        printf("Error reading file.\n");

        if (close(file_descriptor) == -1) {
            printf("Error closing file.\n");
        }

        exit(-1);
    }
    
    printf("\nInput stage\n");

    int number, now_len;
    char* readline_result;

    while (1) {
        readline_result = readline(NULL);

        if (!readline_result) {
            printf("Error: NULL by the readline() function.\n");

            if (close(file_descriptor) == -1) {
                printf("Error closing file.\n");
            }

            exit(-1);
        }
        
        if (!is_digit(readline_result)) {
            printf("Incorrect value input. Try again.\n");

            continue;
        }

        number = atoi(readline_result);

        if (!number) {
            printf("End of input.\n");

            break;
        }

        if (number > now_line) {
            printf("Value out of range [1, %d]. Try again.\n", now_line);

            continue;
        }

        now_len = table[number - 1].len;

        if (lseek(file_descriptor, table[number - 1].offset, SEEK_SET) == -1) {
            printf("Error: -1 by the lseek() function.\n");

            if (close(file_descriptor) == -1) {
                printf("Error closing file.\n");
            }

	    free(readline_result);
            exit(-1);
        }

        while (now_len > 0) {
            if (read(file_descriptor, buffer, minn(sizeof(buffer) - 1, now_len)) == -1) {
                printf("Error reading file.\n");

                if (close(file_descriptor) == -1) {
                    printf("Error closing file.\n");
                }

		free(readline_result);
                exit(-1);
            }

            buffer[minn(sizeof(buffer) - 1, now_len)] = 0;

            printf("%s", buffer);

            now_len -= BUFSIZ;
        }

        printf("\n");
    }

    free(readline_result);

    if (close(file_descriptor) == -1) {
        printf("Error closing file.\n");
        exit(-1);
    }

    exit(0);
}
