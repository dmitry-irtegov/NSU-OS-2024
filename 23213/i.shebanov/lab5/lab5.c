#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

typedef struct Table {
    size_t size;
    size_t capacity;
    off_t * array;
} Table;

Table * createTable() {
    Table * table = (Table *)malloc(sizeof(Table));
    table->capacity = 2;
    table->size = 0;
    table->array = (off_t*)malloc(sizeof(off_t) * table->capacity);
    return table;
}

void add_offset(Table* table, off_t offset) {
    table->array[table->size] = offset;
    table->size += 1;
    if (table->size == table->capacity) {
        table->capacity *= 2;
        table->array = (off_t*)realloc(table->array, sizeof(off_t) * table->capacity);
    }
}

void free_table(Table * table) {
    free(table->array);
    free(table);
}



int main(int argc, char *argv[]){
    if (argc != 2) {
        fprintf(stderr, "Filename is not provided");
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "File cannot be opened");
        return 1;
    }


    off_t file_size = lseek(fd, 0, SEEK_END);
    if(file_size == -1) {
        fprintf(stderr, "Failed lseek");
        close(fd);
        return 1;
    }
    if(lseek(fd, 0, SEEK_SET) == -1) {
        fprintf(stderr, "Failed lseek");
        close(fd);
        return 1;
    }


    Table * offsetTable = createTable();
    add_offset(offsetTable, -1);

    int BUFFER_SIZE = 4;
    char buffer[BUFFER_SIZE];
    int buffers_read = 0;
    
    int bytes_read = 0;
    do {
        bytes_read = read(fd, buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            fprintf(stderr, "Error reading");
            free_table(offsetTable);
            close(fd);
            return 1;
        }
        for(int i = 0; i < bytes_read; i++){
            if(buffer[i] == '\n'){
                add_offset(offsetTable, (off_t)(i + buffers_read * BUFFER_SIZE));
            }
        }
        buffers_read += 1;
    }while (bytes_read > 0);

    add_offset(offsetTable, file_size);

    while(1) {
        int line_number = 0;
        scanf("%d", &line_number);
        if(line_number == 0) {
            free_table(offsetTable);
            close(fd);
            return 0;
        } else if (line_number > 0 && line_number < (int)offsetTable->size){
            int string_length = offsetTable->array[line_number] - offsetTable->array[line_number - 1] - 1;
            char * line_buffer = (char *)malloc(sizeof(char) * (string_length + 1));

            if(!line_buffer){
                fprintf(stderr, "Failed to allocate memory");
                free_table(offsetTable);
                close(fd);
                return 1;
            }

            if(lseek(fd, offsetTable->array[line_number - 1]+1, SEEK_SET) == -1) {
                fprintf(stderr, "Failed lseek");
                free_table(offsetTable);
                free(line_buffer);
                close(fd);
                return 1;
            }
            if(read(fd, line_buffer, string_length) == -1) {
                fprintf(stderr, "Error reading");
                free_table(offsetTable);
                free(line_buffer);
                close(fd);
                return 1;
            };

            line_buffer[string_length] = '\0';
            printf("%s\n", line_buffer); 
            free(line_buffer);
        } else {
            fprintf(stderr, "Invalid line number");
            free_table(offsetTable);
            close(fd);
            return 1;
        }
    }
}
