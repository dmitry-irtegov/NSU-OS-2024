#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

void step(){
    printf("real user id: %d\neffective user id: %d\n", getuid(), geteuid());
    FILE *file = fopen("file.txt", "r");

    if (file == NULL){
        perror("Error cant open file");
    } else {
        fclose(file);
    }

}
int main() {

    step();

    if (setuid(getuid()) == -1){
        perror("Error failed setuid");
    };

    step();

    return(0);
}