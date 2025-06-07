#pragma once

#include "loader/loader.hpp"
#include "loader/nonCachedLoader.hpp"

#include <mutex>
#include <memory>
#include <unordered_map>

#define MAX_CACHABLE_SIZE 2048 //1048576

class LoaderProducer;
class Chunk;
class Request;
class AppendOnlyLog;
class NonCachedLoader;


class CachedLoader : public Loader {
public:
    CachedLoader(std::shared_ptr<Request> request);
    ~CachedLoader();

    void subscribe(int ClientID) override;
    void unsubscribe(int ClientID) override;
    std::shared_ptr<Chunk> getNextChunk(int ClientID) override;
private:
    static void * fetch_entry(void * arg);
    void fetch();
    void setUpLoaders();

    std::unordered_map<int, size_t> subscribers_position_;
    std::unordered_map<int, std::shared_ptr<NonCachedLoader>> noncached_loaders_;
    
    
    std::shared_ptr<Request> request_;
    std::shared_ptr<AppendOnlyLog> cache;
    
    LoaderProducer& loader_producer_;

    int sock_fd_;
    bool failed_;
    bool uncacheable_;
    std::string response;

    pthread_t thread_;
    pthread_mutex_t mutex_clients;
    pthread_mutex_t mutex_block_client;
    pthread_cond_t cond_block_client;
};


enum HttpCheckStatus {
    FAILED,
    PASSED,
    AGAIN,
    STOP
};

HttpCheckStatus checkContentLength(std::string content);
HttpCheckStatus checkStatusCode(std::string content);