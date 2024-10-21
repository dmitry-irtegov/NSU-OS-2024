#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define MAX_LINES 1000 

int Descriptor;
int ifLate = 0;

void printWholeFile() {
    printf("\nToo slow. Printing whole file:\n");

    char buf[BUFSIZ] = {0};

    if (lseek(Descriptor, (off_t) 0, SEEK_SET) == -1){
        perror("Couldn`t lseek");
        return;
    }
    
    ssize_t bytesRead = read(Descriptor, buf, sizeof(buf) - 1);
    if(bytesRead == -1) {
        perror("Couldn`t read from file");
        return;
    }
    

    while (bytesRead != 0) {
        write(STDOUT_FILENO, buf, bytesRead);
        bytesRead = read(Descriptor, buf, sizeof(buf) - 1);
        if(bytesRead == -1) {
            perror("Couldn`t read from file");
            return;
        }
    }
    close(Descriptor);
    return;
}

void sighandler(int sig){
    ifLate = 1;
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
        
        char bufread[BUFSIZ];
        

        int decision;

        alarm(5);

        int scanfReturn = scanf("%d", &decision);

        alarm(0);

        if (ifLate) {
            printWholeFile();
            return 0;
        }
        if (scanfReturn == EOF) {
            if (errno > 0) {
                perror("Couldn`t scanf");
            }
            printf("\n");
            return 1;
        }
        
        if (scanfReturn == 0) {
            scanf("%*[^\n]");
            printf("Wrong. Not number entered\n");
            continue;
        }

        if (decision == 0) {
            break;
        }

        if (decision > nowLine || decision < 0) {
            printf("Wrong. No such line\n");
            continue;
        }

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
