#pragma once

class Proxy {
public:
    Proxy(int port);
    void run();
    

private:
    int proxy_socket_;

    void acceptClient();
    void handleConnection(int client_fd);
};