#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void action(uid_t real, uid_t effective){
    printf("real uid: %d\n", (int) real);
    printf("effective uid: %d\n", (int) effective);

    FILE *file = fopen("file", "r");
    if(file == NULL){
        perror("File wasn't open, possibly you have no rights");
    } else {
       if( fclose(file) != 0){
	   perror("Can not close file");
	}
    }
}

int main() {
    uid_t real = getuid();
    uid_t effective = geteuid();

    action(real,effective);
    if(setuid(real) != 0){
        perror("Can not change uid");
    }
    effective = geteuid();
    action(real, effective);

    exit(0);
}
