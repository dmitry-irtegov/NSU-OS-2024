#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>


void print_user_ids() {
    uid_t real_uid = getuid();  
    uid_t effective_uid = geteuid();  
    printf("Real UID: %d, Effective UID: %d\n", real_uid, effective_uid);
}

int main() {
    const char *filename = "secure_file.txt";

 
    print_user_ids();


    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");  
    } else {
        printf("File '%s' opened successfully.\n", filename);
        fclose(file);  
    }

    
    if (setuid(getuid()) == -1) {
        perror("Error setting user ID");
        exit(EXIT_FAILURE);
    }

  
    print_user_ids();

    
    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");  
    } else {
        printf("File '%s' opened successfully again.\n", filename);
        fclose(file);  
    }

    return 0;
}
