#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

#define BUF_SIZE 1024
char* socket_path = "./socket";

int main() {
  struct sockaddr_un addr;
  int fd, cl;
  ssize_t rc;
  char buf[BUF_SIZE];
  
  if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket error");
    exit(-1);
  }
  
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
  unlink(socket_path);
  
  if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind error");
    exit(-1);
  }
  
  if (listen(fd, 1) == -1) {
    perror("listen error");
    unlink(socket_path);
    exit(-1);
  }
  
  while(1) {
    if ((cl = accept(fd, NULL, NULL)) == -1) {
      perror("accept error");
      continue;
    }
    
    while ((rc = read(cl, buf, sizeof(buf))) > 0) {
      for (int i=0; i < rc; i++) {
        buf[i] = toupper(buf[i]);
      }
      printf("%.*s", rc, buf);
    }
    if (rc == -1) {
      perror("read failed");
      unlink(socket_path);
      exit(-1);
    }
  }
  
  unlink(socket_path);
  exit(0);
}
