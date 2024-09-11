
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>


int main(){
    uid_t real = getuid();
    uid_t eff = geteuid();

    printf("Real ID = %d\n", real);
    printf("Effective ID = %d\n\n", eff);   

    FILE* fp = NULL;
    if (fopen("text3.txt", "r") == NULL) {
        perror("File open error");    
    }
    else {
        printf("First open succeseful\n");
    };

    if (fp != NULL) fclose(fp);

    setuid(real);

    real = getuid();
    eff = geteuid();

    printf("Real ID = %d\n", real);
    printf("Effective ID = %d\n\n", eff);   

    fp = NULL;
    if (fopen("text3.txt", "r") == NULL) {
        perror("File open error");    
    }
    else {
        printf("Second open succeseful\n");
    };

    if (fp != NULL) fclose(fp);

    return 0;
}