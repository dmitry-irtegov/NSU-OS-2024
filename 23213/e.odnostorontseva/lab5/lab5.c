#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFF_SIZE 512
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
    char *line = NULL;
    size_t startedLinesNumber = LINES;
    
    displacements = (off_t*)malloc(startedLinesNumber * sizeof(off_t));
    if (displacements == NULL) {
        perror("Memory allocation error");
        exit(1);
    }

    lines_lens = (off_t*)malloc(startedLinesNumber * sizeof(int));
    if (lines_lens == NULL) {
        perror("Memory allocation error");
        exit(1);
    }
    
    char buf[BUFF_SIZE];
    ssize_t bytes_read = 1;
    displacements[0] = 0;
    while (bytes_read > 0) {
        bytes_read = read(file_descriptor, buf, BUFF_SIZE);
        if(bytes_read == -1)
        {
            perror("Reading error");
            exit(1);
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
                        exit(1);
                    }
                    lines_lens = (off_t*)realloc(lines_lens, startedLinesNumber * sizeof(off_t));
                    if(lines_lens == NULL) {
                        perror("Mempry reallocation error");
                        exit(1);
                    }
                }
                off_t offst = lseek(file_descriptor, 0L, SEEK_CUR) - (bytes_read - i - 1);
                if(offst == -1) {
                    perror("Lseek error");
                    exit(1);
                }
                displacements[current_line] = offst;
            } else {
                symbol_counter++;
            }
        }       
    }

    /*printf("Таблица смещений:\n");
    for (size_t k = 0; k < current_line; k++) {
        printf("Строка %ld: Смещение = %ld, Длина = %ld\n", k + 1, displacements[k], lines_lens[k]);
    }*/

    size_t lineNum = 0;

    while (1) {
        printf("Введите номер строки (0 чтобы завершить): ");
        int res = scanf("%lu", &lineNum);
    
        if (res == EOF) {
            printf("EOF замечен. Завершение программы...\n");
            break;
        }

        if (res != 1) {
            printf("Некорректный ввод. Попробуйте еще раз.\n");
            scanf("%*[^\n]");
            continue;
        }

        if (lineNum == 0) {
            printf("Завершение программы...\n");
            break;
        }

        if (lineNum < 1 || lineNum > current_line) {
            fprintf(stderr, "Неправильно введенный номер строки. Попробуйте еще раз.\n");
            continue;
        }

        if (lseek(file_descriptor, displacements[lineNum - 1], SEEK_SET) == -1) {
            perror("Lseek error");
            exit(1);
        }

        size_t len = lines_lens[lineNum - 1];
        line = (char*)realloc(line, (len + 1) * sizeof(char));
        if (line == NULL) {
            perror("Memory reallocation error");
            exit(1);
        }

        if (read(file_descriptor, line, len) > 0) {
            line[len] = '\0';
            printf("%s", line);
        } else {
            perror("Reading error");
            exit(1);
        }
    }

    free(line);
    free(displacements);
    free(lines_lens);

    if(close(file_descriptor) == -1)
    {
        perror("File closing fail");
        return 1;
    }
    return 0;
}
