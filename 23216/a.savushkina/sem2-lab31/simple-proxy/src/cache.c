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
    for (int i = 0; i < CACHE_SIZE; i++) {
        CacheEntry *entry = cache[i];
        while (entry) {
            printf("URL: %s\n", entry->url);
            printf("Response: %s\n", entry->response);
            entry = entry->next;
        }
    }
}

void add_to_cache(const char *url, const char *response) {
    unsigned long index = hash(url);
    CacheEntry *new_entry = malloc(sizeof(CacheEntry));
    if (!new_entry) {
        return;
    }
    new_entry->url = strdup(url);
    new_entry->response = strdup(response);
    new_entry->timestamp = time(NULL);
    new_entry->next = cache[index];
    cache[index] = new_entry;
    print_cache();
}


void free_cache_entry(CacheEntry *entry) {
    free(entry->url);
    free(entry->response);
    free(entry);
}

int is_cache_entry_expired(CacheEntry *entry, int max_age) {
    time_t current_time = time(NULL);
    return difftime(current_time, entry->timestamp) > max_age;
}

const char *get_from_cache(const char *url) {
    unsigned long index = hash(url);
    CacheEntry *entry = cache[index];
    while (entry) {
        if (strcmp(entry->url, url) == 0) {
            if (is_cache_entry_expired(entry, 60)) {
                cache[index] = entry->next;
                free_cache_entry(entry);
                return NULL;
            }
            return entry->response;
        }
        entry = entry->next;
    }
    

    return NULL;
}

void free_cache() {
    for (int i = 0; i < CACHE_SIZE; i++) {
        CacheEntry *entry = cache[i];
        while (entry) {
            CacheEntry *temp = entry;
            entry = entry->next;
            free(temp->url);
            free(temp->response);
            free(temp);
        }
        cache[i] = NULL;
    }
}