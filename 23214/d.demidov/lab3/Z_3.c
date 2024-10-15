#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    uid_t real_id;
    uid_t effective_id;
    real_id = getuid();
    effective_id = geteuid();
    printf("Real ID = %d\n", real_id);
    printf("Effective ID = %d\n", effective_id);
    FILE* file = fopen("input.txt", "r");
    if (file == NULL) {
        perror("Error opening file input.txt");
        exit(1);
    } 
    fclose(file);
    real_id = getuid();
    setuid(real_id);
    effective_id = geteuid();
    printf("Real ID = %d\n", real_id);
    printf("Effective ID = %d\n", effective_id);
    file = fopen("input.txt", "r");
    if (file == NULL) {
        perror("Error opening file input.txt");
        exit(1);
    } 
    fclose(file);

    return 0;
}
