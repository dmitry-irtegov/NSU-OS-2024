#ifndef CACHE_H
#define CACHE_H

#include <stddef.h>

#define CACHE_SIZE 1024

typedef struct CacheEntry {
    char *url;
    char *response;
    time_t timestamp;
    int max_age;
    int is_complete;
    struct CacheEntry *next;
} CacheEntry;


void add_to_cache(const char *url, const char *response, int max_age);
const char *get_from_cache(const char *url);
void free_cache_entry(CacheEntry *entry);
void mark_cache_entry_complete(const char *url);
void print_cache();
void clean_expired();
#endif