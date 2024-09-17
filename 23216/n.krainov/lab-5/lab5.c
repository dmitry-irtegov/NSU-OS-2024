#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LEN_BUFFER
#define LEN_BUFFER 10
#endif

typedef struct elem_of_vector_off_t{
    off_t off;
    off_t len;
}elem_of_vector_off_t;

typedef struct vector_off_t{
    elem_of_vector_off_t* elems;
    int cur;
    int cap;
}vector_off_t;

int initVector(vector_off_t* ans){
    ans->cur = 0;
    ans->elems = malloc(sizeof(elem_of_vector_off_t)*20);
    if (ans->elems == NULL){
        perror("malloc failed");
    }
    ans->cap = ans->elems == NULL? -1: 20;
    return ans->elems == NULL? 1: 0;
}

int addElem(vector_off_t* vector, off_t off, off_t len){
    if (vector->cap == vector->cur){
        vector->cap *= 2;
        vector->elems = realloc(vector->elems, sizeof(elem_of_vector_off_t) * vector->cap);
        if (vector->elems == NULL){
            perror("realloc failed");
            return 1;
        }
    }
    vector->elems[vector->cur].len = len;
    vector->elems[vector->cur].off = off;
    vector->cur++;
    
    return 0;
}

int searchString(vector_off_t* vector, int num_of_line, int file){
    if (num_of_line >= vector->cur){
        puts("Num of line is too big");
        return 0;
    }
    char* string = calloc(vector->elems[num_of_line-1].len + 1, sizeof(char));
    if (string == NULL){
        perror("calloc failed");
        return 1;
    }

    if (lseek(file, vector->elems[num_of_line-1].off, SEEK_SET) == -1){
        perror("lseek failed");
        return 1;
    }

    if (read(file, string, vector->elems[num_of_line-1].len) == -1){
        perror("read failed");
        return 1;
    }
    puts(string);
    free(string);
    
    return 0;

}

void freeVector(vector_off_t* vector){
    free(vector->elems);
}

int main(int argc, char* argv[]){
    if (argc < 2){
        perror("missing filename");
        exit(EXIT_FAILURE);
    }

    int file = open(argv[1], O_RDONLY);
    if (file == -1){
        perror("open failed");
        exit(EXIT_FAILURE);
    }

    vector_off_t vector;
    if (initVector(&vector)){
        exit(EXIT_FAILURE);
    }
    
    char buffer[LEN_BUFFER];

    ssize_t sym_read;
    off_t cur_len = 0, cur_off = 0;
    char flag = 0;
    while (1){
        sym_read = read(file, buffer, LEN_BUFFER);
        if (sym_read == -1){
            freeVector(&vector);
            perror("read failed");
            exit(EXIT_FAILURE);
        }
        if (sym_read == 0){
            break;
        }
        
        for (ssize_t i = 0; i<sym_read; i++){
            if (buffer[i] == '\n'){
                cur_len++;
                if (addElem(&vector, cur_off, cur_len)){
                    freeVector(&vector);
                    exit(EXIT_FAILURE);
                }
                cur_off+=cur_len;
                cur_len = 0;
            }
            else{
                cur_len++;
            }
        }
        
    }

    if (!flag){
        if (addElem(&vector, cur_off, cur_len)){
            freeVector(&vector);
            exit(EXIT_FAILURE);
        }
    }

    int num_of_line;
    while (1){
        puts("Enter number of string (for end programm enter 0)");
        if(!scanf("%d", &num_of_line) || num_of_line == 0){
            break;
        }
        searchString(&vector, num_of_line, file);
    }

    freeVector(&vector);
    if (close(file) == -1){
        perror("close failed");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);

}