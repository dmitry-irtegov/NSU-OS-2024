#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>


void check_file() {

    real = getuid();
    eff = geteuid();

    if (real == -1) {
        perror("getuid error:");
        return 1;
    }

    if (eff) == -1) {
        perror("geteuid error:");
        return 1;
    }

    printf("Real ID = %d\n", real);
    printf("Effective ID = %d\n\n", eff);

    FILE* fp = NULL;
    if (fopen("text3.txt", "r") == NULL) {
        perror("File open error");
    }

    if (fp != NULL) fclose(fp);
}

int main(){
    
    check_file();

    setuid(real); 

    check_file();


    return 0;
}