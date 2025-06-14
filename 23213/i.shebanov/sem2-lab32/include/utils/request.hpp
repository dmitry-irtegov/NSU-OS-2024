#pragma once

#include <string>
#include <netdb.h>
#include <cstring>
#include <memory>


enum RequestType {
    GET,
    HEAD,
    POST
};

enum RequestValidationErrors {
    VALID_REQUEST,
    UNSUPPORTED_REQUEST_TYPE,
    INVALID_HTTP_VERSION,
    INVALID_HOST,
    INVALID_REQUEST
};

class Request{
public:
    Request(std::string requestText);
    ~Request() = default;
    enum RequestType requestType;
    struct addrinfo * res;
    std::string requestMessage;
    bool isValid;
    enum RequestValidationErrors validationStatus;
    bool operator==(const Request& other) const {
        return requestMessage == other.requestMessage;
    }
private: 
    std::string domain;
    std::string port;
    void splitHost(std::string hostName);
};

struct SharedRequestHash {
    std::size_t operator()(const std::shared_ptr<Request>& r) const {
        return std::hash<std::string>()(r->requestMessage);
    }
};

struct SharedRequestEqual {
    bool operator()(const std::shared_ptr<Request>& a, const std::shared_ptr<Request>& b) const {
        return a->requestMessage == b->requestMessage;
    }
};
