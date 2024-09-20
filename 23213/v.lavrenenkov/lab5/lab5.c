#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char** argv) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s filein \n", argv[0]);
        return 1;
    }
    char* fileName = argv[1];

    int Descriptor = open(fileName, O_RDONLY);

    if(Descriptor == -1) {
        perror("Couldn`t open file");
        return 1;
    }

    off_t offsetsFromStart[1000] = {0};
    off_t lengthsOfLines[1000] = {0};

    char buf[100] = {0};

    ssize_t bytesRead = read(Descriptor, buf, sizeof(buf));

    if(bytesRead == -1) {
        perror("Couldn`t read from file");
        return 1;
    }

    int nowLine = 0;

    while (bytesRead != 0) {
        for (int i = 0; i < bytesRead; i++) {
            if(buf[i] == '\n') {
                nowLine++;
                offsetsFromStart[nowLine] = offsetsFromStart[nowLine - 1] + lengthsOfLines[nowLine - 1] + 1;
            } else {
                lengthsOfLines[nowLine]++;
            }
        }
        bytesRead = read(Descriptor, buf, sizeof(buf));
        if(bytesRead == -1) {
            perror("Couldn`t read from file");
        }
    }
    
    while (1) {
        printf("Write line number(1 - numberOfLinesInFile) or 0 to exit: ");
        int decision;
        while (scanf("%d", &decision) == 0 || decision > nowLine || decision < 0) {
            printf("Wrong. Write line number: ");
            scanf("\n");
        }
        if (decision == 0) {
            break;
        }
        decision -= 1;

        if (lseek(Descriptor, offsetsFromStart[decision], SEEK_SET) == -1){
            perror("Couldn`t lseek");
            return 1;
        }

        off_t lengthOfNeeded = lengthsOfLines[decision]; 

        while (lengthOfNeeded > 0) {
            if(lengthOfNeeded < (off_t) 100) {
                bytesRead = read(Descriptor, buf, lengthOfNeeded);
                if(bytesRead == -1) {
                    perror("Couldn`t read from file");
                    return 1;
                }

                for (int i = 0; i < lengthOfNeeded; i++) {
                    printf("%c", buf[i]);
                }
                
            } else {
                bytesRead = read(Descriptor, buf, sizeof(buf));
                if(bytesRead == -1) {
                    perror("Couldn`t read from file");
                    return 1;
                }

                printf("%s", buf);
            }
            

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
