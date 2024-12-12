#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define   buffer   128

typedef struct elem_of_table_off_t {
    off_t off;
    off_t len;
} elem_of_table_off_t;

typedef struct table_for_file {
    elem_of_table_off_t *elems;
    int cur;
    int cap;
} table_for_file;

int main(int argc, char *argv[]) {
    table_for_file file_table;
    file_table.elems = NULL;
    if (argc != 2) {
        perror("you have to give file name and nothing else");
        exit(EXIT_FAILURE);
    }
    int file = open(argv[1], O_RDONLY);
    if (file == -1){
        perror("problem in file open");
        exit(EXIT_FAILURE);
    }
    file_table.cur = 0;
    file_table.cap = 20;
    file_table.elems = malloc(sizeof(elem_of_table_off_t) * 20);
    if(file_table.elems == NULL){
        perror("problem malloc");
        close(file);
        exit(EXIT_FAILURE);
    }
    char buf[buffer];
    ssize_t read_len;
    off_t cur_len = 0, cur_off = 0;
    while((read_len = read(file,buf,buffer)) >= 0){
        if (read_len == 0){
            break;
        }
        for (ssize_t i = 0; i < read_len; i++) {
            cur_len++;
            if (buf[i] == '\n') {
                if (file_table.cap == file_table.cur) {
                    file_table.cap *= 2;
                    file_table.elems = realloc(file_table.elems, sizeof(elem_of_table_off_t) * file_table.cap);
                    if (file_table.elems == NULL) {
                        perror("problem realloc");
                        close(file);
                        exit(EXIT_FAILURE);
                    }
                }
                file_table.elems[file_table.cur].len = cur_len;
                file_table.elems[file_table.cur].off = cur_off;
                file_table.cur++;
                cur_off += cur_len;
                cur_len = 0;
            }
        }
    }
    if(read_len < 0){
        perror("problem in read");
        free(file_table.elems);
        close(file);
        exit(EXIT_FAILURE);
    }
    if (cur_len != 0){
        if (file_table.cap == file_table.cur) {
            file_table.cap *= 2;
            file_table.elems = realloc(file_table.elems, sizeof(elem_of_table_off_t) * file_table.cap);
            if (file_table.elems == NULL) {
                perror("problem realloc");
                close(file);
                exit(EXIT_FAILURE);
            }
        }
        file_table.elems[file_table.cur].len = cur_len;
        file_table.elems[file_table.cur].off = cur_off;
        file_table.cur++;
        cur_off += cur_len;
        cur_len = 0;
    }
    int num_of_line, res;
    char *string = NULL;
    while (1) {
        puts("Enter number of string (for end programm enter 0)");
        res = scanf("%d", &num_of_line);
        if (res != 1){
            perror("problem in scanf, you should type 1 int number");
            free(file_table.elems);
            close(file);
            exit(EXIT_FAILURE);
        }
        if (num_of_line < 0){
            puts("wrong number");
            continue;
        }else if (num_of_line == 0){
            break;
        }else{
            if (num_of_line > file_table.cur) {
                puts("Num of line is too big");
                continue;
            }
            string = calloc(file_table.elems[num_of_line - 1].len + 1, sizeof(char));
            if (string == NULL) {
                perror("problem calloc");
                free(file_table.elems);
                close(file);
                exit(EXIT_FAILURE);
            }
            if (lseek(file, file_table.elems[num_of_line - 1].off, SEEK_SET) == -1) {
                perror("problem lseek");
                free(file_table.elems);
                free(string);
                close(file);
                exit(EXIT_FAILURE);
            }
            if (read(file, string, file_table.elems[num_of_line - 1].len) == -1) {
                perror("problem lseek");
                free(file_table.elems);
                free(string);
                close(file);
                exit(EXIT_FAILURE);
            }
            printf("%s",string);
        }
        free(string);
    }
    free(file_table.elems);
    close(file);
    exit(EXIT_SUCCESS);
}