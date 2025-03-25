#ifndef UTILS_H
#define UTILS_H

char *extract_host(char *request);

int should_keep_alive(char *request);

int should_cache_response(char *request);

#endif