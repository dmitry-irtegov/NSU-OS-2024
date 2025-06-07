#include "utils/request.hpp"
#include <sstream>
#include <cstring>
#include <iostream>
#include <regex>

Request::Request(std::string requestText) {
    requestMessage = requestText;
    domain = "";
    port = "80";
    isValid = false;

    std::istringstream stream(requestText);
    std::string firstLine;
    if (!std::getline(stream, firstLine, '\r')) {
        validationStatus = INVALID_REQUEST;
        return;
    }
    stream.ignore(1);

    std::istringstream firstLineStream(firstLine);
    std::string method, uri, version;
    if (!(firstLineStream >> method >> uri >> version)) {
        validationStatus = INVALID_REQUEST;
        return;
    }

    if (method == "GET") {
        requestType = GET;
    } else if (method == "HEAD") {
        requestType = HEAD;
    } else if (method == "POST") {
        requestType = POST;
    } else {
        validationStatus = UNSUPPORTED_REQUEST_TYPE;
        return;
    }

    if (version != "HTTP/1.0" && version != "HTTP/1.1") {
        validationStatus = INVALID_HTTP_VERSION;
        return;
    }

    if (uri.rfind("http://", 0) == 0) {
        std::string tmp = uri.substr(7);
        std::size_t slashPos = tmp.find("/");
        if (slashPos == std::string::npos) {
            validationStatus = INVALID_HOST;
            return;
        }
        domain = tmp.substr(0, slashPos);
        std::size_t portPos = domain.find(":");
        if (portPos != std::string::npos) {
            port = domain.substr(portPos + 1);
            domain = domain.substr(0, portPos);
        }
    }

    if (domain.empty()) {
        std::size_t hostPos = requestText.find("Host:");
        if (hostPos == std::string::npos) {
            validationStatus = INVALID_HOST;
            return;
        }

        hostPos += 5;
        std::size_t lineEnd = requestText.find("\r\n", hostPos);
        if (lineEnd == std::string::npos) {
            validationStatus = INVALID_HOST;
            return;
        }

        std::string hostValue = requestText.substr(hostPos, lineEnd - hostPos);
        splitHost(hostValue);
    }

    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = PF_INET;
    if (getaddrinfo(domain.c_str(), port.c_str(), &hints, &res) != 0) {
        validationStatus = INVALID_HOST;
        return;
    }

    std::ostringstream rebuilt;
    rebuilt << method << " " << uri << " HTTP/1.0\r\n";

    std::string headerLine;
    while (std::getline(stream, headerLine, '\r')) {
        stream.ignore(1);
        if (headerLine.empty() || headerLine == "\n")
            break;

        std::string lowerHeader = headerLine;
        std::transform(lowerHeader.begin(), lowerHeader.end(), lowerHeader.begin(), ::tolower);

        if (lowerHeader.find("connection:") == 0 ||
            lowerHeader.find("proxy-connection:") == 0 ||
            lowerHeader.find("te:") == 0 ||
            lowerHeader.find("transfer-encoding:") == 0 ||
            lowerHeader.find("expect:") == 0 ||
            lowerHeader.find("upgrade:") == 0) {
            continue;
        }

        rebuilt << headerLine << "\r\n";
    }

    rebuilt << "Connection: close\r\n\r\n";
    requestMessage = rebuilt.str();

    isValid = true;
    validationStatus = VALID_REQUEST;
}

void Request::splitHost(std::string hostName) {
    std::size_t portStart = hostName.find(":");
    if (portStart != std::string::npos) {
        domain = hostName.substr(0, portStart);
        port = hostName.substr(portStart + 1);
    } else {
        domain = hostName;
        port = "80";
    }
}
