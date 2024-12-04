#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    
    printf("Real user ID: %d\n", getuid());
    printf("Effective user ID: %d\n", geteuid());

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opened");
    } else {
        printf("Successfully opened \n");
        if (fclose(file) == EOF) {
            perror("Error closing file");
        }
    }

    // 使用真实用户 ID 设置有效用户 ID
    if (seteuid(getuid()) == -1) {
        perror("Set effective UID");
    }

    // 再次打印用户 ID
    printf("Real user ID: %d\n", getuid());
    printf("Effective user ID: %d\n", geteuid());

    // 再次尝试打开文件
    file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opened");
    } else {
        printf("File opened successfully after UID change\n");
        if (fclose(file) == EOF) {
            perror("Error closing file after UID change");
        }
    }

    return 0;
}
