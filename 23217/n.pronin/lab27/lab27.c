#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 2) { 
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) { 
        perror("Could not open file");
        return EXIT_FAILURE;
    }

    FILE *pipe = popen("wc -l", "w");
    if (!pipe) { // Проверяем успешность открытия pipe
        perror("Could not open pipe");
        fclose(file);
        return EXIT_FAILURE;
    }

    char *buffer = NULL; 
    size_t buffer_size = 0;

    // Считываем файл построчно
    while (getline(&buffer, &buffer_size, file) != -1) {
        // Проверяем, что строка пуста
        if (buffer[0] == '\n') {
            if (fputs(buffer, pipe) == EOF) { // Пишем пустую строку в pipe
                perror("Error writing to pipe");
                free(buffer);
                fclose(file);
                pclose(pipe);
                return EXIT_FAILURE;
            }
        }
    }

    free(buffer);
    fclose(file);

    if (pclose(pipe) == -1) {
        perror("Error closing pipe");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
