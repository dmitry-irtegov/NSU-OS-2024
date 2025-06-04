#pragma once

#include "utils/request.hpp"

#include <unordered_map>
#include <memory>
#include <pthread.h>

class CachedLoader;
class Loader;

class LoaderProducer {
public:
    static LoaderProducer& instance();

    std::shared_ptr<Loader> getLoader(std::shared_ptr<Request> request);
    void removeHashedLoader(std::shared_ptr<Request> request);
    void setUnhashable(std::shared_ptr<Request> request);

private:
    LoaderProducer();
    ~LoaderProducer();

    LoaderProducer(const LoaderProducer&) = delete;
    LoaderProducer& operator=(const LoaderProducer&) = delete;

    std::unordered_map<std::shared_ptr<Request>, std::shared_ptr<CachedLoader>, SharedRequestHash, SharedRequestEqual> cache_map;
    std::unordered_map<std::shared_ptr<Request>, bool, SharedRequestHash, SharedRequestEqual> noncaching_map;

    pthread_mutex_t mutex;
};
