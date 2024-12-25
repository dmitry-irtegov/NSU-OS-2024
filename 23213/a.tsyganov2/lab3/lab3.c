#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    uid_t real_id = getuid();
    uid_t effective_id = geteuid();
    
    printf("real id: %d\neffective id: %d\n", real_id, effective_id);
    
    FILE* file = fopen("datafile", "r");
    if(file == NULL) {
        perror("fopen error\n");
    } else {
        printf("fopen success\n");
        fclose(file);
    }
    
    if(setuid(real_id) == -1) {
        perror("setuid error\n");
        return EXIT_FAILURE;
    }
    
    real_id = getuid();
    effective_id = geteuid();
    
    printf("setuid success\n");
    printf("real id: %d\neffective id: %d\n", real_id, effective_id);
    
    file = fopen("datafile", "r");
    if(file == NULL) {
        perror("fopen error\n");
    } else {
        printf("fopen success\n");
        fclose(file);
    }
    
    return EXIT_SUCCESS;
}
