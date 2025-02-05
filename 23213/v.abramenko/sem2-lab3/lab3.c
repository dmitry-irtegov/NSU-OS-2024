#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>


void* thread_body(void* param) {
    for(char** t =(char **)param; *t!=NULL; t++) {
        printf("%s\n", *t);
    }
    return NULL;
}

void my_perror(int code, int argc, char **argv) {
    char buf[256];
    strerror_r(code, buf, sizeof(buf));
    fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
}

int main(int argc, char *argv[]) {
    pthread_t thread[4];
    int code;
    static char *param_1[]={ "Thread 1 line 1", "Thread 1 line 2", "thread 1 line 3", NULL };
    static char *param_2[]={ "Thread 2 line 1", "Thread 2 line 2", "thread 2 line 3", "Thread 2 line 4", NULL };
    static char *param_3[]={ "Thread 3 line 1", "Thread 3 line 2", "thread 3 line 3", NULL };
    static char *param_4[]={ "Thread 4 line 1", "Thread 4 line 2", NULL };
        
    if ((code=pthread_create(&thread[0], NULL, thread_body, param_1))!=0) {
        my_perror(code, argc, argv);
        exit(-1);
    }
    
    if ((code=pthread_create(&thread[1], NULL, thread_body, param_2))!=0) {
        my_perror(code, argc, argv);
        exit(-1);
    }
    
    if ((code=pthread_create(&thread[2], NULL, thread_body, param_3))!=0) {
        my_perror(code, argc, argv);
        exit(-1);
    }
    
    if ((code=pthread_create(&thread[3], NULL, thread_body, param_4))!=0) {
        my_perror(code, argc, argv);
        exit(-1);
    }
    printf("Parent\n");
    pthread_exit(NULL);
}
