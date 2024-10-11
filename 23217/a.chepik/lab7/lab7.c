#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define minn(a,b) (((a) < (b)) ? (a) : (b))
#define TABLE_SIZE 2048

int file_descriptor = 0;
char* file_mapping = NULL, *readline_result = NULL;
off_t now_indx = 0, file_size = 0;

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

void handler(int signum) {
    int fd = file_descriptor;
    char* rdline = readline_result;

    const char close_msg[21] = "Error closing file.\n";

    char buffer[BUFSIZ];
    off_t now_size = file_size;
    now_indx = 0;

    while (now_size > 0) {
        write(STDOUT_FILENO, file_mapping + now_indx, minn(sizeof(buffer), now_size));
        now_indx += minn(sizeof(buffer), now_size), now_size -= BUFSIZ;
    }

    if (close(fd) == -1) {
        write(STDOUT_FILENO, close_msg, 21);
    }

    free(rdline);
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("File name not found in argv.\n");
        exit(-1);
    }

    const char* filename = argv[1];

    file_descriptor = open(filename, O_RDONLY);
    if (file_descriptor == -1) {
        printf("Error opening file.\n");
        exit(-1);
    }

    struct stat file_stat;
    if (fstat(file_descriptor, &file_stat) == -1) {
        printf("Error: -1 by the fstat() function.\n");

        if (close(file_descriptor) == -1) {
            printf("Error closing file.\n");
        }

        exit(-1);
    }

    file_size = file_stat.st_size;

    file_mapping = (char*)mmap(NULL, (size_t)file_size, PROT_READ, MAP_PRIVATE, file_descriptor, 0);
    if (file_mapping == MAP_FAILED) {
        printf("Error: MAP_FAILED.");

        if (close(file_descriptor) == -1) {
            printf("Error closing file.\n");
        }

        exit(-1);
    }

    signal(SIGALRM, handler);

    char buffer[BUFSIZ];
    int now_line = 0, pos = 0, len = 0, flag = 1;

    ssize_t read_result;
    line table[TABLE_SIZE] = { 0 };

    table[now_line].offset = (off_t)0;

    off_t now_size = file_size;
    while (flag && now_size > 0) {
        memcpy(buffer, file_mapping + now_indx, minn(sizeof(buffer) - 1, now_size));

        for (now_indx = 0; now_indx < minn(sizeof(buffer) - 1, now_size); now_indx++) {
            pos++, len++;

            if (buffer[now_indx] == '\n') {
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

        now_indx += minn(sizeof(buffer) - 1, now_size), now_size -= BUFSIZ;
    }

    printf("\nInput stage\n");

    int number, now_len;

    while (1) {
        alarm(5);
        readline_result = readline(NULL);
        alarm(0);

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

        now_len = table[number - 1].len, now_indx = table[number - 1].offset;

        while (now_len > 0) {
            memcpy(buffer, file_mapping + now_indx, minn(sizeof(buffer) - 1, now_len));
            buffer[minn(sizeof(buffer) - 1, now_len)] = 0;

            printf("%s", buffer);

            now_indx += minn(sizeof(buffer) - 1, now_len), now_len -= BUFSIZ;
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
