
#include "client.hpp"
#include "proxy.hpp"

#include<sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#define BACKLOG 10

Proxy::Proxy(int port) {
    proxy_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (proxy_socket_ < 0) {
        throw std::system_error(errno, std::system_category(), "socket creation failed");
    }

    struct sockaddr_in listen_addr;
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(port);
    listen_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(proxy_socket_, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0) {
        throw std::system_error(errno, std::system_category(), "bind failed");
    }

    if(listen(proxy_socket_, BACKLOG)) {
        throw std::system_error(errno, std::system_category(), "listen failed");
    }
} 

void Proxy::run() {
    std::cout << "Proxy started" << std::endl;
    while(true) {
        int client_socket;
        if((client_socket = accept(proxy_socket_, NULL, NULL))<0) {
            std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
        }

        std::cout << "Connected" << std::endl;
        Client::create(client_socket);
    }
}

