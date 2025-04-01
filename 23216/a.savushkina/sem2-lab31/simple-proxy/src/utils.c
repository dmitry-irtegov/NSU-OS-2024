#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cache.h"


char *extract_host(char *request) {
    char *host_header = strstr(request, "Host: ");
    if (!host_header) {
        return NULL;
    }
    host_header += 6; 
    char *host_end = strstr(host_header, "\r\n");
    if (!host_end) {
        return NULL;
    }
    size_t host_length = host_end - host_header;
    char *host = (char *)malloc(host_length + 1);
    if (!host) {
        return NULL;
    }
    strncpy(host, host_header, host_length);
    host[host_length] = '\0';
    return host;
}

int should_keep_connection(char *request) {
    char *protocol_get = strstr(request, "GET");
    char *protocol_head = strstr(request, "HEAD");
    if (!protocol_get && !protocol_head){
        return 0;
    }
    return 1;
}

int should_cache_response(char *request) {
    char *cache_control_header = strstr(request, "Cache-Control: ");
    if (!cache_control_header) {
        return 1;
    }
    cache_control_header += 15;
    if (strncmp(cache_control_header, "no-cache", 8) == 0) {
        return 0;
    }
    return 1;
}