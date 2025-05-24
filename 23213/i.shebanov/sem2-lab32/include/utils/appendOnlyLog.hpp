#pragma once

#include <list>
#include <memory>
#include <pthread.h>

#include <iostream>

class Chunk;

class AppendOnlyLog {
public:
    AppendOnlyLog() {
        pthread_mutex_init(&mutex, nullptr);
    }

    ~AppendOnlyLog() {
        pthread_mutex_destroy(&mutex);
    }

    void append(std::shared_ptr<Chunk> chunk) {
        pthread_mutex_lock(&mutex);
        chunks.push_back(chunk);
        pthread_mutex_unlock(&mutex);
    }

    void push_front(std::shared_ptr<Chunk> chunk){
        pthread_mutex_lock(&mutex);
        chunks.push_front(chunk);
        pthread_mutex_unlock(&mutex);
    }

    std::shared_ptr<Chunk> getChunk(size_t index) {
        pthread_mutex_lock(&mutex);
        std::shared_ptr<Chunk> chunk = nullptr;
        if (index < chunks.size()) {
            auto it = std::next(chunks.begin(), index);
            chunk = *it;
        }
        pthread_mutex_unlock(&mutex);
        return chunk;
    }


    size_t size() {
        pthread_mutex_lock(&mutex);
        size_t s = chunks.size();
        pthread_mutex_unlock(&mutex);
        return s;
    }

private:
    std::list<std::shared_ptr<Chunk>> chunks;
    pthread_mutex_t mutex;
};
