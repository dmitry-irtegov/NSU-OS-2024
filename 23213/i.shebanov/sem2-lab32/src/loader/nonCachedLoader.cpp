#include "loader/nonCachedLoader.hpp"
#include "chunk.hpp"
#include "utils/request.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <system_error>
#include <pthread.h>
#include <iostream>



NonCachedLoader::NonCachedLoader(std::shared_ptr<Request> request, int pad) : request_(request), pad_(pad){
    sock_fd_ = socket(request_->res->ai_family,
        request_->res->ai_socktype,
        request_->res->ai_protocol);

    if (sock_fd_ == -1) {
        perror("socket");
        throw std::runtime_error("Socket creation failed");
    }

    struct timeval tv = {30, 0};
    if (setsockopt(sock_fd_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) == -1) {
        perror("setsockopt");
        close(sock_fd_);
        throw std::runtime_error("Setsockopt failed");
    }

    if (sem_init(&producer_, 0, 1) == -1) {
        perror("producer semaphore");
        close(sock_fd_);
        throw std::runtime_error("Failed to create producer semaphore");
    }
    if (sem_init(&consumer_, 0, 0) == -1) {
        perror("consumer semaphore");
        close(sock_fd_);
        sem_destroy(&producer_);
        throw std::runtime_error("Failed to create consumer semaphore");
    }

    if (pthread_create(&thread_, NULL, NonCachedLoader::fetch_entry, this) == -1){
        perror("pthread_create");
        close(sock_fd_);
        sem_destroy(&consumer_);
        sem_destroy(&producer_);
        throw std::runtime_error("Failed to create fetch thread");
    }
}

NonCachedLoader::~NonCachedLoader() {
    sem_destroy(&producer_);
    sem_destroy(&consumer_);
    pthread_cancel(thread_);
    close(sock_fd_);
}


void* NonCachedLoader::fetch_entry(void* arg) {
    NonCachedLoader* self = static_cast<NonCachedLoader*>(arg);
    self->fetch();
    return nullptr;
}

void NonCachedLoader::fetch() {
    if (connect(sock_fd_, request_->res->ai_addr, request_->res->ai_addrlen) == -1) {
        perror("noncached connection failed");
        close(sock_fd_);
        next_chunk_ = std::make_shared<Chunk>(nullptr, 0, true, true);;
        sem_post(&consumer_);
        return;
    }

    const std::string &msg = request_->requestMessage;
    size_t total_size = msg.size();
    size_t sent = 0;

    while (sent < total_size)
    {
        size_t to_send = std::min(static_cast<size_t>(1024), total_size - sent);
        ssize_t bytes_written = write(sock_fd_, msg.data() + sent, to_send);

        if (bytes_written == -1)
        {
            perror("write failed");
            close(sock_fd_);
            next_chunk_ = std::make_shared<Chunk>(nullptr, 0, true, true);
            return;
        }

        sent += bytes_written;
    }

    while (true) {
        sem_wait(&producer_);

        char buffer[1024];
        ssize_t read_size = read(sock_fd_, buffer, sizeof(buffer));;

        if (read_size <= 0) {
            next_chunk_ = std::make_shared<Chunk>(buffer, read_size, true, read_size < 0);
            sem_post(&consumer_);
            return;
        }

        if(pad_ >= read_size) {
            sem_post(&producer_);
            pad_ -= read_size;
        }else {
            memmove(buffer, buffer + pad_, read_size - pad_);
            read_size -= pad_;
            pad_ = 0;
            next_chunk_ = std::make_shared<Chunk>(buffer, read_size, false, false);
            sem_post(&consumer_);
        }

    }
}

std::shared_ptr<Chunk> NonCachedLoader::getNextChunk(int clientID) {
    sem_wait(&consumer_);
    std::shared_ptr<Chunk> chunk_pointer_copy = next_chunk_;
    sem_post(&producer_);
    return chunk_pointer_copy;
}

void NonCachedLoader::subscribe(int ClientID) {}
void NonCachedLoader::unsubscribe(int ClientID) {
    pthread_cancel(thread_);
}