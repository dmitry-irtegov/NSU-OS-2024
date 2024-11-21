#include <sys/socket.h>
#include <sys/un.h>

const struct sockaddr_un sock_addr = {AF_UNIX, "mysocket"};
const int sock_addr_len = 14;
