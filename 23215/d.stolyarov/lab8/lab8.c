#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      
#include <string.h>
#include <unistd.h>                   
int main(int argc, char *argv[]){

    if(argc == 1){
        perror("Missing arguement");
        exit(1);
    }
    int file;                   
    if((file = open(argv[1], O_RDWR)) == -1){
        perror("File opening error");
        exit(2);        
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(file, F_SETLK , &lock) == -1) {
        perror("Locking error");
        exit(3);
    }

    char cmd[100] = "nano " ;
    int offset = strlen(cmd);
    int fileNameSize = strlen(argv[1]);
    for(int i = 0; i < fileNameSize; i++){
        cmd[offset + i] = argv[1][i];   
    }
    cmd[offset + fileNameSize] = 0;
    
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