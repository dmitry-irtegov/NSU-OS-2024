#include <stdio.h>
#include <stdlib.h>

void count_empty_lines(FILE *inputFile, FILE *wcPipe) {
    char *line = NULL;
    size_t len = 0;

    // Читаем строки из входного файла и записываем пустые строки в конвейер
    while (getline(&line, &len, inputFile) != -1) {
        if (line[0] == '\n') {
            fputs(line, wcPipe);
        }
    }

    // Освобождаем память
    free(line);
}


int main(int argc, char** argv) {
    FILE *inputFile;
    FILE *wcPipe;//output file

    // Проверяем, передан ли аргумент с именем файла
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // Открываем входной файл
    if ((inputFile = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", argv[1]);
        return 1;
    }

    // Открываем конвейер для команды wc
    if ((wcPipe = popen("wc -l", "w")) == NULL) {
        fprintf(stderr, "Failed to open pipe to wc\n");
        fclose(inputFile);
        return 1;
    }

    // Вызываем функцию для подсчета пустых строк
    count_empty_lines(inputFile, wcPipe);

    // Закрываем файлы и конвейер
    fclose(inputFile);
    pclose(wcPipe);
    return 0;
}