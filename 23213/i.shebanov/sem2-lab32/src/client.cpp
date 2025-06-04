#include "client.hpp"
#include "utils/request.hpp"
#include "loader/loader.hpp"
#include "loader/loaderProducer.hpp"
#include "chunk.hpp"

#include <iostream>
#include <thread>
#include <unistd.h>
#include <memory>

void Client::create(int fd) {
    Client* client = new Client(fd);
    pthread_t thread;

    if(pthread_create(&thread, NULL, runEntry, client) == -1) {
        perror("Thread creationg failed");
        delete client;
    }
}

Client::Client(int fd) : fd_(fd) {
}

Client::~Client() {
    close(fd_);
}

void * Client::runEntry(void*client) {
    Client *self = static_cast<Client *>(client);
    self->run();
    return nullptr;
}

void Client::run() {
    std::string requestText;
    char buf[1024];
    while (requestText.find("\r\n\r\n") == std::string::npos) {
        ssize_t n = read(fd_, buf, sizeof(buf));
        if (n <= 0) {
            delete this;
            return;
        }
        requestText.append(buf, n);
    }
    std::shared_ptr<Request> request = std::make_shared<Request>(requestText);
    if(request->requestType == POST) {
        std::string bodyText;
        auto it = requestText.find("Content-Length:");
        if (it != std::string::npos) {
            size_t start = it + 15;
            while (start < requestText.size() && std::isspace(requestText[start])) ++start;
            size_t end = requestText.find("\r\n", start);
            size_t content_length = std::stoul(requestText.substr(start, end - start));
            size_t header_end = requestText.find("\r\n\r\n") + 4;
            bodyText = requestText.substr(header_end);
            while (bodyText.size() < content_length) {
                ssize_t n = read(fd_, buf, sizeof(buf));
                if (n <= 0) {
                    delete this;
                    return;
                }
                bodyText.append(buf, n);
            }
        }
        request->requestMessage += bodyText;
    }
    


    if(request->validationStatus == INVALID_HTTP_VERSION) {
        const char* version_not_supported_response =
            "HTTP/1.0 505 HTTP Version Not Supported\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 35\r\n"
            "Connection: close\r\n"
            "\r\n"
            "Received unsupported HTTP version.";

        size_t response_length = strlen(version_not_supported_response);
        write(fd_, version_not_supported_response, response_length);
        delete this;
        return;
    }

    if(request->isValid == false) {
        const char* bad_request_response =
            "HTTP/1.0 400 Bad Request\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 24\r\n"
            "Connection: close\r\n"
            "\r\n"
            "Request is invalid.";

        size_t response_length = strlen(bad_request_response);
        write(fd_, bad_request_response, response_length);
        delete this;
        return;
    }


    std::shared_ptr<Loader> loader;
    try{
        LoaderProducer& producer = LoaderProducer::instance();
        loader = producer.getLoader(request);
    } catch (...) {
        close(fd_);
        delete this;
        return;
    }
    loader->subscribe(fd_);
    while(true) {
        auto chunk = loader->getNextChunk(fd_);
        if (!chunk->isError) {
            const char* data_ptr = chunk->data();
            size_t total_to_write = chunk->size();
            size_t written = 0;
        
            while (written < total_to_write) {
                ssize_t n = write(fd_, data_ptr + written, total_to_write - written);
                if (n < 0) {
                    perror("write error");
                    break;
                }
                written += static_cast<size_t>(n);
            }
        }

        if (chunk->isError == true || chunk->isFinal == true) {
            loader->unsubscribe(fd_);
            break;
        }
    }

    delete this;
}
