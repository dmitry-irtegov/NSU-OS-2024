#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>


uid_t check_file() {

    uid_t real = getuid();
    uid_t eff = geteuid();

    if (real == -1) {
        perror("getuid error:");
        return -1;
    }

    if (eff == -1) {
        perror("geteuid error:");
        return -1;
    }

    printf("Real ID = %d\n", real);
    printf("Effective ID = %d\n\n", eff);

    FILE* fp = NULL;
    if (fopen("text3.txt", "r") == NULL) {
        perror("File open error");
    }

    if (fp != NULL) fclose(fp);
    
    return real;
}

int main(){
    
    uid_t real;
    if ((real = check_file()) == -1) {
        return 1;
    }


    setuid(real); 

    if ((real = check_file()) == -1) {
        return 1;
    }


    return 0;
}