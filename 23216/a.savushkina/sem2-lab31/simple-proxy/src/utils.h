#ifndef UTILS_H
#define UTILS_H

char *extract_host(char *request);

int should_cache_response(char *request);

int should_keep_connection(char *request);

int content_length_provided(char *response);

int enough_memory_for_cache(size_t response_length);

#endif