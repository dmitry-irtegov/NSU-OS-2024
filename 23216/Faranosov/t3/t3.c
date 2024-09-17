#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>


void check_file() {

    FILE* fp = NULL;
    if (fopen("text3.txt", "r") == NULL) {
        perror("File open error");
    }
    else {
        printf("First open succeseful\n");
    }

    if (fp != NULL) fclose(fp);
}

int main(){
    uid_t real = getuid();
    uid_t eff = geteuid();
    
    if (real == -1) {
        perror("getuid1 error:");
        return 1;
    }

    if (eff) == -1) {
        perror("geteuid1 error:");
        return 1;
    }

    printf("Real ID = %d\n", real);
    printf("Effective ID = %d\n\n", eff);   

    check_file();

    setuid(real);

    real = getuid();
    eff = geteuid();

    if (real == -1) {
        perror("getuid2 error:");
        return 1;
    }

    if (eff) == -1) {
        perror("geteuid2 error:");
        return 1;
    }

    printf("Real ID = %d\n", real);
    printf("Effective ID = %d\n\n", eff);   

    check_file();


    return 0;
}