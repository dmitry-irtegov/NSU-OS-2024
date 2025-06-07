#pragma once

class Client {
public:
    static void create(int fd);

private:
    Client(int fd);
    ~Client();

    static void * runEntry(void * client);
    void run();

    int fd_;
};
