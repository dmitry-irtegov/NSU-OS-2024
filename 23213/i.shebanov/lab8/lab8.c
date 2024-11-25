#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char * argv[]){
        if(argc == 1)
        {
                fprintf(stderr, "No file is specified");
                exit(1);
        }

        int fd = open(argv[1], O_RDWR);
        if (fd == -1)
        {
                perror("Could not open the file");
                exit(1);
        }

        struct flock lock;
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = 0;
        lock.l_len = 0;
        lock.l_pid = getpid();

        if(fcntl(fd, F_SETLK, &lock) == -1)
        {
                perror("Failed to set lock");
                exit(1);
        }


        char command[128];
        sprintf(command, "vim %s\n", argv[1]);

        if (system(command) == -1) 
        {
            printf("System command: fail.\n");
            exit(-1);
        }

        lock.l_type = F_UNLCK;
        if(fcntl(fd, F_SETLK, &lock) == -1)
        {
                perror("Failed to unlock");
                exit(1);
        }
        return 0;
}