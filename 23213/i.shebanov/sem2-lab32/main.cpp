#include "proxy.hpp"

#include <signal.h>
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    signal(SIGPIPE, SIG_IGN);
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    int proxy_port = std::atoi(argv[1]);
    if (proxy_port <= 0) {
        std::cerr << "Invalid port number" << std::endl;
        return 1;
    }

    Proxy server(proxy_port);
    server.run();
    return 0;
}
