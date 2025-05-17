#include "loader/loaderProducer.hpp"
#include "loader/cachedLoader.hpp"
#include "loader/nonCachedLoader.hpp"

#include <memory>
#include <stdexcept>
#include <iostream>

LoaderProducer& LoaderProducer::instance() {
    static LoaderProducer instance;
    return instance;
}

LoaderProducer::LoaderProducer() {
    if (pthread_mutex_init(&mutex, nullptr) != 0) {
        perror("loader producer mutex");
        throw std::runtime_error("Failed to create loader producer mutex");
    }
}

LoaderProducer::~LoaderProducer() {
    pthread_mutex_destroy(&mutex);
}

std::shared_ptr<Loader> LoaderProducer::getLoader(std::shared_ptr<Request> request) {
    pthread_mutex_lock(&mutex);

    if(request->requestType == POST) {
        pthread_mutex_unlock(&mutex);
        try{
            return std::static_pointer_cast<Loader>(std::make_shared<NonCachedLoader>(request));
        } catch (...) {
            throw;
        }
    }

    if (noncaching_map.find(request) != noncaching_map.end()) {
        pthread_mutex_unlock(&mutex);
        try{
            return std::static_pointer_cast<Loader>(std::make_shared<NonCachedLoader>(request));
        } catch (...) {
            throw;
        }
    }

    if (cache_map.find(request) != cache_map.end()) {
        std::shared_ptr<CachedLoader> loader = cache_map[request];
        pthread_mutex_unlock(&mutex);
        return std::static_pointer_cast<Loader>(loader);
    }

    std::shared_ptr<CachedLoader> cached_loader;
    try {
        cached_loader = std::make_shared<CachedLoader>(request);
    } catch (...) {
        throw;
    }
    cache_map[request] = cached_loader;
    pthread_mutex_unlock(&mutex);
    return std::static_pointer_cast<Loader>(cached_loader);
}

void LoaderProducer::removeHashedLoader(std::shared_ptr<Request> request) {
    pthread_mutex_lock(&mutex);
    cache_map.erase(request);
    pthread_mutex_unlock(&mutex);
}

void LoaderProducer::setUnhashable(std::shared_ptr<Request> request) {
    pthread_mutex_lock(&mutex);
    noncaching_map[request] = true;
    pthread_mutex_unlock(&mutex);
}
