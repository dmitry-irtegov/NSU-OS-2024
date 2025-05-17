#pragma once

class Client {
public:
    static void create(int fd);

private:
    explicit Client(int fd);
    ~Client();

    static void * runEntry(void * client);
    void run();

    int fd_;
};
