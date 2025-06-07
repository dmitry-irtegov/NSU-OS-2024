#include "loader/cachedLoader.hpp"
#include "loader/loaderProducer.hpp"
#include "utils/appendOnlyLog.hpp"
#include "chunk.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <system_error>
#include <pthread.h>

#include <iostream>

CachedLoader::CachedLoader(std::shared_ptr<Request> request)
    : request_(request),
      cache(std::make_shared<AppendOnlyLog>()),
      loader_producer_(LoaderProducer::instance()),
      failed_(false),
      uncacheable_(false),
      response()
{
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


    if (pthread_mutex_init(&mutex_clients, nullptr) == -1) {
        perror("clients mutex");
        close(sock_fd_);
        throw std::runtime_error("Failed to create clients mutex");
    }
    if (pthread_mutex_init(&mutex_block_client, nullptr) == -1) {
        perror("block mutex");
        close(sock_fd_);
        pthread_mutex_destroy(&mutex_clients);
        throw std::runtime_error("Failed to create block mutex");
    }
    if (pthread_cond_init(&cond_block_client, nullptr) == -1) {
        perror("block cond var");
        close(sock_fd_);
        pthread_mutex_destroy(&mutex_clients);
        pthread_mutex_destroy(&mutex_block_client);
        throw std::runtime_error("Failed to create block condition variable");
    }

    if (pthread_create(&thread_, NULL, CachedLoader::fetch_entry, this) == -1) {
        perror("pthread_create");
        close(sock_fd_);
        pthread_mutex_destroy(&mutex_clients);
        pthread_mutex_destroy(&mutex_block_client);
        pthread_cond_destroy(&cond_block_client);
        throw std::runtime_error("Failed to create fetch thread");
    }
}

CachedLoader::~CachedLoader(){
    pthread_mutex_destroy(&mutex_clients);
    pthread_mutex_destroy(&mutex_block_client);
    pthread_cond_destroy(&cond_block_client);
    close(sock_fd_);
}

void *CachedLoader::fetch_entry(void *arg)
{
    CachedLoader *self = static_cast<CachedLoader *>(arg);
    self->fetch();
    return nullptr;
}

void CachedLoader::fetch()
{
    if (connect(sock_fd_, request_->res->ai_addr, request_->res->ai_addrlen) == -1)
    {
        perror("connection failed");
        cache->append(std::make_shared<Chunk>(nullptr, 0, true, true));
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
            cache->append(std::make_shared<Chunk>(nullptr, 0, true, true));
            return;
        }

        sent += bytes_written;
    }

    HttpCheckStatus length_check = AGAIN;
    HttpCheckStatus http_status_code_check = AGAIN;
    while (true)
    {
        char buffer[1024];
        ssize_t read_size = read(sock_fd_, buffer, sizeof(buffer));

        std::shared_ptr<Chunk> next_chunk;
        if (read_size <= 0)
        {
            if (read_size < 0) {
                loader_producer_.removeHashedLoader(request_);
            }
            next_chunk = std::make_shared<Chunk>(nullptr, 0, true, read_size < 0);
            cache->append(next_chunk);
            pthread_mutex_lock(&mutex_block_client);
            pthread_cond_broadcast(&cond_block_client);
            pthread_mutex_unlock(&mutex_block_client);
            break;
        }
        else
        {
            next_chunk = std::make_shared<Chunk>(buffer, read_size, false, false);
            cache->append(next_chunk);
            pthread_mutex_lock(&mutex_block_client);
            pthread_cond_broadcast(&cond_block_client);
            pthread_mutex_unlock(&mutex_block_client);
        }

        response.append(buffer, read_size);


        if(http_status_code_check == AGAIN && (http_status_code_check = checkStatusCode(response)) == FAILED) {
            loader_producer_.removeHashedLoader(request_);
        }

        if (length_check == AGAIN && ((length_check = checkContentLength(response)) == FAILED))
        {
            loader_producer_.setUnhashable(request_);
            loader_producer_.removeHashedLoader(request_);
            setUpLoaders();
            uncacheable_ = true;
            break;
        }
    }
}

void CachedLoader::setUpLoaders()
{
    size_t skip_size = response.size();
    for (const auto &pair : subscribers_position_)
    {
        noncached_loaders_[pair.first] = std::make_shared<NonCachedLoader>(request_, skip_size);
        noncached_loaders_[pair.first]->subscribe(pair.first);
    }

    pthread_mutex_lock(&mutex_block_client);
    pthread_cond_broadcast(&cond_block_client);
    pthread_mutex_unlock(&mutex_block_client);
}

std::shared_ptr<Chunk> CachedLoader::getNextChunk(int clientID)
{
    pthread_mutex_lock(&mutex_clients);
    size_t client_position = subscribers_position_[clientID];
    subscribers_position_[clientID] += 1;
    pthread_mutex_unlock(&mutex_clients);
    if(client_position < cache->size()) {
        return cache->getChunk(client_position);
    }

    if(uncacheable_) {
        pthread_mutex_lock(&mutex_clients);
        pthread_mutex_unlock(&mutex_clients);
        return noncached_loaders_[clientID]->getNextChunk(clientID);
    }

    pthread_mutex_lock(&mutex_block_client);
    while (client_position >= cache->size() && !uncacheable_)
    {
        pthread_cond_wait(&cond_block_client, &mutex_block_client);
    }
    pthread_mutex_unlock(&mutex_block_client);


    if (uncacheable_ == false) {
        return cache->getChunk(client_position);
    }
    else {
        pthread_mutex_lock(&mutex_clients);
        pthread_mutex_unlock(&mutex_clients);
        return noncached_loaders_[clientID]->getNextChunk(clientID);
    }
}

void CachedLoader::subscribe(int ClientID)
{
    pthread_mutex_lock(&mutex_clients);
    subscribers_position_[ClientID] = 0;
    pthread_mutex_unlock(&mutex_clients);
}
void CachedLoader::unsubscribe(int ClientID)
{
    pthread_mutex_lock(&mutex_clients);
    subscribers_position_.erase(ClientID);
    pthread_mutex_unlock(&mutex_clients);
}


HttpCheckStatus checkStatusCode(std::string content) {
    size_t first_line_end = content.find("\r\n");
    if (first_line_end == std::string::npos)
    {
        return AGAIN;
    }

    if (content.substr(0, first_line_end).find("200") == std::string::npos)
    {
        return FAILED;
    }

    return PASSED;
}

HttpCheckStatus checkContentLength(std::string content)
{
    if (content.length() > MAX_CACHABLE_SIZE)
    {
        return FAILED;
    }

    size_t headers_end = content.find("\r\n\r\n");
    if (headers_end != std::string::npos)
    {
        return AGAIN;
    }
    size_t content_length_pos = content.find("Content-Length: ");
    if (content_length_pos == std::string::npos)
    {
        return AGAIN;
    }

    std::string tmp = "Content-Length: ";
    size_t len_content = tmp.size();
    content.erase(content_length_pos + len_content);
    size_t length;
    if ((length = content.find("\r\n")) == std::string::npos)
    {
        return AGAIN;
    }

    int content_length;
    try
    {
        content_length = std::stoi(content.substr(0, length));
    }
    catch (...)
    {
        return FAILED;
    }

    if (content_length > MAX_CACHABLE_SIZE)
    {
        return FAILED;
    }

    return PASSED;
}