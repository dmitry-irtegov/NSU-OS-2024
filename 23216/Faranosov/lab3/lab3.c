#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>


uid_t check_file() {

    uid_t real = getuid();
    uid_t eff = geteuid();


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
    real = check_file();


    setuid(real); 

    check_file();


    exit(0);
}
