#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void* thread_function() {
  for (int i = 0; i < 10; i++) {
    printf("Hello World\n");
  }
  return NULL;
}
int main() {
  pthread_t thread;
  if (pthread_create(&thread, NULL, thread_function, NULL) != 0) {
    perror("error creating thread");
    exit(EXIT_FAILURE);
  }
  thread_function(NULL);

  pthread_exit(NULL);

}