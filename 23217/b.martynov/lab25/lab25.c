#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFET_SIZE 1024

int main()
{
    int file_des[2] = { 0 };
    if (pipe(file_des) == -1) {
        perror("pipe() unsuccess");
        exit(EXIT_FAILURE);
    }

    pid_t fork_res = fork();

    if (fork_res == -1) {
        perror("fork() unsuccess");
        exit(EXIT_FAILURE);
    }

    unsigned char bufet[] = "SASha was WaLkInG on HIGHway and SUCKed BAGel.";
    int flag = 0;

    if (fork_res == 0) {
        if (close(file_des[0]) == -1) {
            perror("Can't close file_des[0]");
            flag = 1;
        }

        ssize_t write_res = write(file_des[1], (const void*)bufet, BUFET_SIZE);
        if (write_res == -1) {
            perror("write() unsuccess");
            flag = 1;
        }

        if (close(file_des[1])) {
            perror("Can't close file_des[1]");
            flag = 1;
        }

        if (flag) {
            exit(EXIT_FAILURE);
        }
    }
    else {
        if (close(file_des[1]) == -1) {
            perror("Can't close file_des[1]");
            flag = 1;
        }

        unsigned char bufet_for_read[BUFET_SIZE] = { 0 };

        ssize_t read_res = read(file_des[0], (void*)bufet_for_read, BUFET_SIZE);
        if (read_res == -1) {
            perror("read() unsuccess");
            flag = 1;
        }

        if (close(file_des[0])) {
            perror("Can't close file_des[0]");
            flag = 1;
        }

        if (flag) {
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < BUFET_SIZE; i++) {
            bufet_for_read[i] = toupper((int)(bufet_for_read[i]));
        }

        printf("%s\n", bufet_for_read);
    }

    return 0;
}
