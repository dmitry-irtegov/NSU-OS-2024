#ifndef CACHE_H
#define CACHE_H

#include <stddef.h>

#define CACHE_SIZE 1024

typedef struct CacheEntry {
    char *url;
    char *response;
    time_t timestamp;
    struct CacheEntry *next;
} CacheEntry;


void add_to_cache(const char *url, const char *response);
const char *get_from_cache(const char *url);
void free_cache();
#endif