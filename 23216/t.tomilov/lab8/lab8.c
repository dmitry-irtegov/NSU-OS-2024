#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

void printTime(time_t curTime, time_t startTime){
    int time = (int)difftime(curTime, startTime);
    if (time / 3600 > 0){
        printf("\r%d:%d:%d", time / 3600, (time % 3600) / 60, time % 60);
    } 
    else if (time / 60 > 0){
        printf("\r%d:%d", (time % 3600) / 60, time % 60);
    } 
    else{
        printf("\r%d", time % 60);
    }
    fflush(stdout);
}

void waitForUnlock(int fd){
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    time_t startTime = time(NULL);

    if (fcntl(fd, F_GETLK, &lock) == -1){
        perror("ERROR: Failed to check file lock!");
        close(fd);
        _exit(EXIT_FAILURE);
    }
    if (lock.l_type != F_UNLCK){
        printf("Waiting for the file to be unlocked...\n");
        printTime(time(NULL), startTime);
        sleep(1);
        while (1){
            if (fcntl(fd, F_GETLK, &lock) == -1){
                perror("ERROR: Failed to check file lock!");
                close(fd);
                _exit(EXIT_FAILURE);
            }
            if (lock.l_type == F_UNLCK){
                break;
            }
            else{
                printTime(time(NULL), startTime);
            }
            sleep(1);
        }
    }
}

int main(int argc, char** argv){
    if (argc != 2){
        fprintf(stderr, "Wrong format: try %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct flock lock;

    int fd = open(argv[1], O_RDWR);
    if (fd == -1){
        perror("ERROR: File doesn't exist or access denied!");
        exit(EXIT_FAILURE);
    }

    waitForUnlock(fd);

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(fd, F_SETLKW, &lock) == -1){
        perror("ERROR: Failed to lock the file!");
        close(fd);
        exit(EXIT_FAILURE);
    }

    char* nano = malloc(strlen("nano ") + strlen(argv[1]) + 1);
    if (!nano){
        perror("ERROR: Memory allocation failed!");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (snprintf(nano, strlen("nano ") + strlen(argv[1]) + 1, "nano %s", argv[1]) < 0){
        perror("ERROR: Failed to prepare command!");
        free(nano);
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (system(nano) == -1){
        perror("ERROR: Failed to execute nano!");
        free(nano);
        close(fd);
        exit(EXIT_FAILURE);
    }

    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &lock) == -1){
        perror("ERROR: Failed to unlock the file!");
        free(nano);
        close(fd);
        exit(EXIT_FAILURE);
    }

    free(nano);
    close(fd);
    return EXIT_SUCCESS;
}
