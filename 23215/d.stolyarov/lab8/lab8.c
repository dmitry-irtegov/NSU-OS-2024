#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      
#include <string.h>
#include <unistd.h>
int main(int argc, char *argv[]){

    if(argc < 3){
        perror("Missing arguements");
        exit(1);
    }
    int file;                   
    if((file = open(argv[2], O_RDWR)) == -1){
        perror("File opening error");
        exit(2);        
    }
    else{
        printf("File opened successfully\n");
    }

    struct flock lock;

    if(strcmp(argv[1], "0") == 0){
        lock.l_type = F_RDLCK;
    }
    else if(strcmp(argv[1], "1") == 0){
        lock.l_type = F_WRLCK;
    }
    else{
        perror("Lock type error");
        exit(7);
    }
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(file, F_SETLK , &lock) == -1) {
        perror("Locking error");
        exit(3);
    }
    else{
        printf("Locked successfully\n");
    }


    if(strlen(argv[2]) >= 95){
        perror("File length error");
        exit(6);
    }
    char cmd[100] = "nano " ;
    strcat(cmd, argv[2]);
    
    if(system(cmd) == -1){
        perror("Editing error");
        exit(4);
    }

    lock.l_type = F_UNLCK;
    if(fcntl(file, F_SETLK, &lock) == -1){
        perror("Unlocking error");
        exit(5);
    }

    close(file);
    exit(0);
}   
