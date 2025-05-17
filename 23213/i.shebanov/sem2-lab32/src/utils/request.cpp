#include "utils/request.hpp"

#include <memory>
#include <iostream>


Request::Request(std::string requestText) {
    requestMessage = requestText;
    domain = "";

    std::size_t firstLineEnd = requestText.find("\r\n");
    if(firstLineEnd == std::string::npos) {
        isValid = false;
        validationStatus = INVALID_REQUEST;
        return;
    }
    std::string firstLine = requestText.substr(0, firstLineEnd);

    std::size_t requestTypeEnd = firstLine.find(" ");
    if(requestTypeEnd == std::string::npos) {
        isValid = false;
        validationStatus = INVALID_REQUEST; 
        return;
    }
    std::string requestType = firstLine.substr(0, requestTypeEnd);

    if(requestType == "GET"){
        requestType = GET;
    } else if (requestType == "HEAD") {
        requestType = HEAD;
    } else if (requestType == "POST") {
        requestType = POST;
    } else {
        validationStatus = UNSUPPORTED_REQUEST_TYPE;
        isValid = false;
        return;
    }

    firstLine.erase(0, requestTypeEnd);
    if(firstLine.find(" ") == std::string::npos) {
        isValid = false;
        validationStatus = INVALID_REQUEST;
        return;
    }
    firstLine.erase(0, 1);

    std::size_t uriEnd = firstLine.find(" ");
    if (uriEnd == std::string::npos) {
        validationStatus = INVALID_REQUEST;
        isValid = false;
        return;
    }
    std::string uri = firstLine.substr(0, uriEnd);
    
    if(uri.rfind("http://", 0) != std::string::npos) {
        std::string tmp = "http://";
        std::string uriCopy = uri.erase(0, tmp.length());
        std::size_t hostEnd = uriCopy.find("/");
        if(hostEnd == std::string::npos) {
            validationStatus = INVALID_HOST;
            isValid = false;
            return;
        }
        domain = uriCopy.substr(0, hostEnd);
        
        std::size_t portStart = domain.find(":");
        if(portStart != std::string::npos) {
            port = domain.erase(0,portStart);
            domain = domain.substr(0, portStart);
        } else {
            port = "80";
        }
    }

    firstLine.erase(0, uriEnd);
    if(firstLine.length() == 0) {
        validationStatus = INVALID_HTTP_VERSION;
        isValid = false;
        return;
    }
    firstLine.erase(0, 1);
    std::string protocolType = firstLine;
    
    if(protocolType != "HTTP/1.0"){
        validationStatus = INVALID_HTTP_VERSION;
        isValid = false;
        return;
    }


    if(domain == ""){

        std::size_t hostHeaderStart =  requestText.find("Host: ");
        if(hostHeaderStart == std::string::npos) {
            validationStatus = INVALID_HOST;
            isValid = false;
            return;
        }

        std::size_t hostHeaderEnd = requestText.find("\r\n", hostHeaderStart);
        std::string tmp = "Host: ";
        std::string host = requestText.substr(hostHeaderStart + tmp.length(), hostHeaderEnd-hostHeaderStart-tmp.length());
        Request::splitHost(host);
    }

    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = PF_INET;

    if(getaddrinfo(domain.c_str(), port.c_str(), &hints, &res) != 0){
        validationStatus = INVALID_HOST;
        isValid = false;
        return;
    }

    isValid = true;
    validationStatus = VALID_REQUEST;
}


void Request::splitHost(std::string hostName) {
    std::size_t portStart = hostName.find(":");
    if(portStart != std::string::npos) {
        domain = hostName.substr(0, portStart);
        port = hostName.erase(portStart, std::string::npos);
    } else {
        domain = hostName;
        port = "80";
    }
}