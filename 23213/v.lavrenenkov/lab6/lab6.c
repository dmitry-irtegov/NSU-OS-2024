#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#define MAX_LINES 1000 

int Descriptor;

void sighandler(int sig){
    write(STDOUT_FILENO, "\nToo slow. Printing whole file:\n", 33);

    char buf[BUFSIZ] = {0};

    if (lseek(Descriptor, (off_t) 0, SEEK_SET) == -1){
        write(STDERR_FILENO, "Couldn`t lseek\n", 16);
        _exit(1);
    }
    
    ssize_t bytesRead = read(Descriptor, buf, sizeof(buf) - 1);
    if(bytesRead == -1) {
        write(STDERR_FILENO, "Couldn`t read from file\n", 25);
        _exit(1);
    }
    

    while (bytesRead != 0) {
        write(STDOUT_FILENO, buf, bytesRead);
        bytesRead = read(Descriptor, buf, sizeof(buf) - 1);
        if(bytesRead == -1) {
            write(STDERR_FILENO, "Couldn`t read from file\n", 25);
            _exit(1);
        }

    }
    close(Descriptor);
    _exit(0);
}

int main(int argc, char** argv) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s filein \n", argv[0]);
        return 1;
    }
    char* fileName = argv[1];

    Descriptor = open(fileName, O_RDONLY);

    if(Descriptor == -1) {
        perror("Couldn`t open file");
        return 1;
    }

    off_t offsetsFromStart[MAX_LINES] = {0};
    off_t lengthsOfLines[MAX_LINES] = {0};

    char buf[BUFSIZ] = {0};

    ssize_t bytesRead = read(Descriptor, buf, sizeof(buf));
    if(bytesRead == -1) {
        perror("Couldn`t read from file");
        return 1;
    }

    int nowLine = 0;
    
    int flagForLast = 0; //to check if \n exists in last possible line 
    while (bytesRead != 0) {
        for (int i = 0; i < bytesRead; i++) {
            if(flagForLast == 1) { // if after last line the are some symbols => file is too large
                fprintf(stderr, "Too much lines. Exit\n");
                return 1;
            }
            if(buf[i] == '\n') {
                if(nowLine == MAX_LINES - 1) {
                    nowLine++;
                    flagForLast = 1;
                } else {
                    nowLine++;
                    offsetsFromStart[nowLine] = offsetsFromStart[nowLine - 1] + lengthsOfLines[nowLine - 1] + 1;
                }
                
            } else {
                lengthsOfLines[nowLine]++;
            }
        }
        bytesRead = read(Descriptor, buf, sizeof(buf));
        if(bytesRead == -1) {
            perror("Couldn`t read from file");
            return 1;
        }
    }
    
    signal(SIGALRM, sighandler);
    
    while (1) {
        printf("Write line number(1 - numberOfLinesInFile) or 0 to exit: ");
        
        alarm(5);

        int decision;

        int scanfReturn = scanf("%d", &decision);
        if (scanfReturn == EOF) {
            printf("\n");
            return 1;
        }
        
        if (scanfReturn == 0) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Wrong. Not number entered\n");
            continue;
        }

        if (decision == 0) {
            break;
        }

        

        if (decision > nowLine || decision < 0) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Wrong. No such line\n");
            continue;
        }

        alarm(0);

        decision -= 1;

        if (lseek(Descriptor, offsetsFromStart[decision], SEEK_SET) == -1){
            perror("Couldn`t lseek");
            return 1;
        }

        off_t lengthOfNeeded = lengthsOfLines[decision]; 

        while (lengthOfNeeded > 0) {
            bytesRead = read(Descriptor, buf, sizeof(buf) - 1);
            if(bytesRead == -1) {
                perror("Couldn`t read from file");
                return 1;
            }

            int positionOfEOL = sizeof(buf) - 1;

            if (lengthOfNeeded < (off_t) (sizeof(buf) - 1)) {
                positionOfEOL = (int) lengthOfNeeded;
            }
            
            buf[positionOfEOL] = '\0';
            printf("%s", buf);
            lengthOfNeeded -= bytesRead;
        }
        printf("\n");
    }
    if(close(Descriptor) == -1){
        perror("Couldn`t close file");
        return 1;
    }
    return 0;

}
