#pragma once

#include "loader/loader.hpp"

#include <semaphore.h>
#include <unordered_map>
#include <memory>

class Chunk;
class Request;

class NonCachedLoader : public Loader {
public:
    NonCachedLoader(std::shared_ptr<Request> request, int pad=0);
    ~NonCachedLoader();

    void subscribe(int ClientID) override;
    void unsubscribe(int ClientID) override;
    std::shared_ptr<Chunk> getNextChunk(int ClientID) override;
private:
    static void * fetch_entry(void * arg);
    void fetch();

    std::shared_ptr<Chunk> next_chunk_;
    std::shared_ptr<Request> request_;
    
    int sock_fd_;
    int pad_;

    pthread_t thread_;
    sem_t producer_;
    sem_t consumer_;
};