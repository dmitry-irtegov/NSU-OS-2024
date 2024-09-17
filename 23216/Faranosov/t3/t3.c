#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>


int check_file() {

    uid_t real = getuid();
    uit_t eff = geteuid();

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
    
    return 0
}

int main(){
    
    if (check_file()) {
        return 1;
    }

    uid_t real = getuid();
    if (real == -1) {
        perror("getuid error:");
        return 1;
    }
    setuid(real); 

    if (check_file()) {
        return 1;
    }


    return 0;
}