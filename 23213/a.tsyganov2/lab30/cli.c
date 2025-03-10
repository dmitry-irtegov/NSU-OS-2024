#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define BUF_SIZE 1024
char* socket_path = "./socket";

int main() {
  struct sockaddr_un addr;
  int fd;
  ssize_t rc;
  char buf[BUF_SIZE];
  
  if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket error");
    exit(-1);
  }
  
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
  
  if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("connect error");
    exit(-1);
  }
  
  while( (rc = read(STDIN_FILENO, buf, sizeof(addr))) > 0) {
    if(write(fd, buf, rc) != rc) {
      if (rc > 0) {
        fprintf(stderr, "partial write");
      } else {
        perror("write error");
        exit(-1);
      }
    }
  }
  
  exit(0);
}

