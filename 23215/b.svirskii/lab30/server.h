#include <sys/socket.h>
#include <sys/un.h>

static const struct sockaddr_un sock_addr = { AF_UNIX, "mysocket" };
static const int sock_addr_len = 14;
