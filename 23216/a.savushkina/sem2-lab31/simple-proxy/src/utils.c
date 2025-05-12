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

char *extract_path(char *request) {
    const char *path_start = strstr(request, "://");
    if (path_start) {
        path_start = strchr(path_start + 3, '/');
    } else {
        path_start = strchr(request, '/');
    }

    if (!path_start) {
        return strdup("/");
    }
    const char *path_end = strstr(path_start, " ");
    if (path_end) {
        size_t path_length = path_end - path_start;
        char *path = (char *)malloc(path_length + 1);
        if (!path) {
            return NULL;
        }
        strncpy(path, path_start, path_length);
        path[path_length] = '\0';
        return path;
    }
    size_t path_length = strlen(path_start);
    char *path = (char *)malloc(path_length + 1);
    if (!path) {
        return NULL;
    }

    return strdup(path_start);
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
        return 60;
    }
    cache_control_header += 15;

    if (strncmp(cache_control_header, "no-cache", 8) == 0) {
        return 0;
    }

    char *max_age_str = strstr(cache_control_header, "max-age=");
    if (max_age_str) {
        max_age_str += 8;
        char *max_age_end = strpbrk(max_age_str, ", \r\n");
        size_t max_age_length = max_age_end ? (size_t)(max_age_end - max_age_str) : strlen(max_age_str);
        char max_age_value[16] = {0};
        strncpy(max_age_value, max_age_str, max_age_length);
        return atoi(max_age_value);
    }

    return 60;
}

int content_length_provided(char *response) {
    char *content_length_header = strstr(response, "Content-Length: ");
    if (!content_length_header) {
        return 0;
    }
    content_length_header += 16; 
    char *content_length_end = strstr(content_length_header, "\r\n");
    if (!content_length_end) {
        return 0;
    }
    size_t content_length = content_length_end - content_length_header;
    char *length_str = (char *)malloc(content_length + 1);
    if (!length_str) {
        return 0;
    }
    strncpy(length_str, content_length_header, content_length);
    length_str[content_length] = '\0';
    int length = atoi(length_str);
    free(length_str);
    return length;
}

int enough_memory_for_cache(size_t response_length) {
    size_t available_memory = 1024 * 1024 * 10;
    return response_length < available_memory;
}

int extract_status_code(char *response) {
    char *status_line = strstr(response, "HTTP/");
    if (!status_line) {
        return -1;
    }
    int status_code;
    if (sscanf(status_line, "HTTP/%*s %d", &status_code) == 1) {
        return status_code;
    }
    return -1;
}