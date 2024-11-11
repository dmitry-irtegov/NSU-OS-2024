#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() 
{
    int* file_des = {0, 0};
    if (pipe(file_des) == -1) {
        perror("pipe() unsuccess");
        exit(EXIT_FAILURE);
    }

    pid_t fork_res = fork();

    if (fork_res == -1) {
        perror("fork() unsuccess");
        exit(EXIT_FAILURE);
    }
    
    const size_t bufet_size = 47;
    int flag = 0;

    if (fork_res == 0) {
        if (close(file_des[0]) == -1) {
            perror("Can't close file_des[0]");
            flag = 1;
        }
        
        unsigned char bufet[bufet_size] = "SASha was WaLkInG on HIGHway and SUCKed BAGel.";
        
        ssize_t write_res = write(file_des[1], (const void*)bufet, bufet_size * sizeof(char));
        if (write_res == -1) {
            perror("write() unsuccess");
            flag = 1;
        }
        else if (write_res < bufet_size) {
            printf("write() didn't write the whole sentence\n");
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

        unsigned char bufet[bufet_size] = { 0 };

        ssize_t read_res = read(file_des[0], (void*)bufet, bufet_size);
        if (read_res == -1) {
            perror("read() unsuccess");
            flag = 1;
        }
        else if (read_res < bufet_size) {
            printf("read() didn't read the whole sentence\n");
            flag = 1;
        }

        if (close(file_des[0])) {
            perror("Can't close file_des[0]");
            flag = 1;
        }

        if (flag) {
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < bufet_size; i++) {
            bufet[i] = toupper((int)bufet[i]);
        }

        printf("%s\n", bufet);
    }
}