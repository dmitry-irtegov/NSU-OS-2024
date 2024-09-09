#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int main(void){

    uid_t real_id = getuid();
    uid_t effective_id = geteuid();

    printf("real id: %d\neffective id: %d\n", real_id, effective_id);

    FILE* file = fopen("file", "r");
    if (file == NULL){
        perror("Error while opening file");
    }
    else{
        fclose(file);
    }

    if (setuid(real_id) == -1)
        perror("Error setting id");
    
    effective_id = geteuid();
    
    printf("real id: %d\neffective id: %d\n", real_id, effective_id);

    file = fopen("file", "r");
    if (file == NULL){
        perror("Error while opening file");
    }
    else{
        fclose(file);
    }

    return 0;
}
