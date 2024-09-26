#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/mman.h>

#ifndef LEN_BUFFER
#define LEN_BUFFER 10
#endif

char* fileContent;
struct stat buf;

typedef struct elem_of_vector_off_t{
    off_t off;
    off_t len;
}elem_of_vector_off_t;

typedef struct vector_off_t{
    elem_of_vector_off_t* elems;
    int cur;
    int cap;
}vector_off_t;

int file;
char* filename;
vector_off_t vector;

int initVector(){
    vector.cur = 0;
    vector.elems = malloc(sizeof(elem_of_vector_off_t)*20);
    vector.cap = vector.elems == NULL? -1: 20;
    return vector.elems == NULL? 1: 0;
}

int addElem(off_t off, off_t len){
    if (vector.cap == vector.cur){
        vector.cap *= 2;
        vector.elems = realloc(vector.elems, sizeof(elem_of_vector_off_t) * vector.cap);
        if (vector.elems == NULL){
            return 1;
        }
    }
    vector.elems[vector.cur].len = len;
    vector.elems[vector.cur].off = off;
    vector.cur++;

    return 0;
}

int searchString(int num_of_line, int file){
    if (num_of_line >= vector.cur){
        puts("Num of line is too big");
        return 0;
    }
    if (write(STDOUT_FILENO, (fileContent + vector.elems[num_of_line-1].off), vector.elems[num_of_line-1].len) == -1){
        return 1;
    }
    
    return 0;

}

void handler(){

    if (write(STDOUT_FILENO, fileContent, buf.st_size) == -1 || write(STDOUT_FILENO, "\n", 1) == -1){
        write(STDERR_FILENO, "write failed", 12);
        _exit(EXIT_FAILURE);
    }
    _exit(EXIT_SUCCESS);
}

void exitProgram(int Code, char* message){
    if (message != NULL){
        perror(message);
    }
    exit(Code);
}

int main(int argc, char* argv[]){
    if (argc < 2){
        fprintf(stderr, "missing argument");
        exitProgram(EXIT_FAILURE, NULL);
    }

    signal(SIGALRM, handler);

    filename = argv[1];
    file = open(filename, O_RDONLY);
    if (file == -1){
        exitProgram(EXIT_FAILURE, "open failed");
    }

    if (initVector(&vector)){
        exitProgram(EXIT_FAILURE, "initVector failed");
    }

    off_t cur_len = 0, cur_off = 0;
    if (stat(argv[1], &buf) == -1){
        exitProgram(EXIT_FAILURE, "stat failed");
    }
    fileContent = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, file, 0);
    if (fileContent == MAP_FAILED){
        exitProgram(EXIT_FAILURE, "mmap failed");
    }
    for (off_t i = 0; i < buf.st_size; i++){
        if (fileContent[i] == '\n'){
            cur_len++;
            if (addElem(cur_off, cur_len)){
                exitProgram(EXIT_FAILURE, "addElem failed");
            }
            cur_off+=cur_len;
            cur_len = 0;
        }
        else{
            cur_len++;
        }
    }

    int num_of_line, res;
    while (1){
        puts("Enter number of string (for end programm enter 0)");
        alarm(5);
        res = scanf("%d", &num_of_line);
        alarm(0);
        if (res == EOF){
            if (feof(stdin)) {
                fprintf(stderr, "EOF\n");
                exitProgram(EXIT_FAILURE, NULL);
            }
            else{
                exitProgram(EXIT_FAILURE, "scanf failed");
            }
        }
        else if (res == 0) {
            while(getc(stdin) != '\n'){
                if (feof(stdin)) {
                    fprintf(stderr, "EOF\n");
                    exitProgram(EXIT_FAILURE, NULL);
                }
                else{
                    exitProgram(EXIT_FAILURE, "getc failed");
                }
            }
            continue;
        }
        else if (num_of_line == 0){
            break;
        }
        searchString(num_of_line, file);
    }
    exitProgram(EXIT_SUCCESS, NULL);
}