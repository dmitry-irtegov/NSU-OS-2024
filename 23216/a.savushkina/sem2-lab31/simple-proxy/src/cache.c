#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cache.h"

static CacheEntry *cache[CACHE_SIZE];

unsigned long hash(const char *url) {
    unsigned long hash = 5381;
    int c;

    while ((c = *url++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash % CACHE_SIZE;
}

void print_cache() {
    printf("Cache contents:\n");
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache[i] != NULL) {
            printf("Index %d:\n", i);
        }
        CacheEntry *entry = cache[i];
        int count = 0;
        while (entry) {
            printf("URL: %s\n", entry->url);
            printf("Response: %s\n", entry->response);
            entry = entry->next;
            count++;
        }
        if (count == 0) {
        }
        else {
            printf("Cache contains %d entries\n", count);
        }
    }
    printf("End of cache\n");
}

void add_to_cache(const char *url, const char *response, int max_age) {
    if (max_age <= 0) {
        printf("Invalid max_age for caching: %d\n", max_age);
        return;
    }
    unsigned long index = hash(url);
    CacheEntry *entry = cache[index];

    while (entry) {
        if (strcmp(entry->url, url) == 0) {
            if (!entry->is_complete) {
                printf("Resource is already being cached: %s\n", url);
                return;
            }
        }
        entry = entry->next;
    }

    CacheEntry *new_entry = malloc(sizeof(CacheEntry));
    if (!new_entry) {
        perror("Failed to allocate memory for cache entry");
        return;
    }
    new_entry->url = strdup(url);
    new_entry->response = strdup(response);
    if (!new_entry->url || !new_entry->response) {
        perror("Failed to duplicate URL or response");
        free_cache_entry(new_entry);
        return;
    }
    new_entry->timestamp = time(NULL);
    new_entry->max_age = max_age;
    new_entry->is_complete = 0;
    new_entry->next = cache[index];
    cache[index] = new_entry;
    printf("Resource added to cache: %s\n", url);
}

void free_cache_entry(CacheEntry *entry) {
    free(entry->url);
    free(entry->response);
    free(entry);
}

int is_cache_entry_expired(CacheEntry *entry) {
    time_t current_time = time(NULL);
    return difftime(current_time, entry->timestamp) > entry->max_age;
}

int time_to_expire(const char *url) {
    unsigned long index = hash(url);

    CacheEntry *entry = cache[index];
    while (entry) {
        if (strcmp(entry->url, url) == 0) {
            return entry->max_age - difftime(time(NULL), entry->timestamp);
        }
        entry = entry->next;
    }
    time_t current_time = time(NULL);
    return entry->max_age - difftime(current_time, entry->timestamp);
}

const char *get_from_cache(const char *url) {
    unsigned long index = hash(url);
    CacheEntry *entry = cache[index];
    while (entry) {
        if (strcmp(entry->url, url) == 0) {
            if (!entry->is_complete) {
                printf("Resource is still being cached: %s\n", url);
                return NULL;
            }
            if (is_cache_entry_expired(entry)) {
                printf("Cache entry expired: %s\n", url);
                cache[index] = entry->next;
                free_cache_entry(entry);
                return NULL;
            }
            printf("Cache hit: %s\n", url);
            return entry->response;
        }
        entry = entry->next;
    }
    printf("Cache miss: %s\n", url);
    return NULL;
}

void mark_cache_entry_complete(const char *url) {
    unsigned long index = hash(url);
    CacheEntry *entry = cache[index];
    while (entry) {
        if (strcmp(entry->url, url) == 0) {
            entry->is_complete = 1;
            return;
        }
        entry = entry->next;
    }
}

void free_cache() {
    for (int i = 0; i < CACHE_SIZE; i++) {
        CacheEntry *entry = cache[i];
        while (entry) {
            CacheEntry *temp = entry;
            entry = entry->next;
            free_cache_entry(temp);
        }
        cache[i] = NULL;
    }
}