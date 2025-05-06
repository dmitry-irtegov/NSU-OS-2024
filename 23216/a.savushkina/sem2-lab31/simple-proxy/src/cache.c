#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cache.h"

static CacheEntry *cache[CACHE_SIZE];

unsigned long hash(const char *url) {
    unsigned long h = 5381;
    int c;
    while ((c = *url++)) {
        h = ((h << 5) + h) + c;
    }
    return h % CACHE_SIZE;
}

void print_cache() {
    printf("\n\n\nCache contents:\n\n\n");
    for (int i = 0; i < CACHE_SIZE; i++) {
        CacheEntry *entry = cache[i];
        if (!entry) continue;
        printf("Index %d:\n", i);
        while (entry) {
            printf("  URL: %s (age %ld/%d, complete=%d)\n",
                   entry->url,
                   (long)difftime(time(NULL), entry->timestamp),
                   entry->max_age,
                   entry->is_complete);
            entry = entry->next;
        }
    }
    printf("\n\n\nEnd of cache\n\n\n");
}

void add_to_cache(const char *url, const char *response, int max_age) {
    if (max_age <= 0) return;

    unsigned long idx = hash(url);
    CacheEntry *entry = cache[idx];

    while (entry) {
        if (strcmp(entry->url, url) == 0) {
            if (entry->is_complete) {
                free(entry->response);
                entry->response = strdup(response);
                entry->timestamp = time(NULL);
                entry->max_age = max_age;
                entry->is_complete = 0;
            }
            return;
        }
        entry = entry->next;
    }

    CacheEntry *new_entry = malloc(sizeof(CacheEntry));
    if (!new_entry) return;

    new_entry->url = strdup(url);
    new_entry->response = strdup(response);
    new_entry->timestamp = time(NULL);
    new_entry->max_age = max_age;
    new_entry->is_complete = 0;
    new_entry->next = cache[idx];
    cache[idx] = new_entry;
}

void free_cache_entry(CacheEntry *entry) {
    if (!entry) return;
    free(entry->url);
    free(entry->response);
    free(entry);
}

int is_cache_entry_expired(CacheEntry *entry) {
    return difftime(time(NULL), entry->timestamp) > entry->max_age;
}


void clean_expired(){
    for (int i = 0; i < CACHE_SIZE; i++) {
        CacheEntry *prev = NULL;
        CacheEntry *entry = cache[i];
        while (entry) {
            if (is_cache_entry_expired(entry)) {
                if (prev) prev->next = entry->next;
                else cache[i] = entry->next;
                free_cache_entry(entry);
                entry = prev ? prev->next : cache[i];
            } else {
                prev = entry;
                entry = entry->next;
            }
        }
    }
}

const char *get_from_cache(const char *url) {
    unsigned long idx = hash(url);
    CacheEntry *prev = NULL;
    CacheEntry *entry = cache[idx];

    while (entry) {
        if (strcmp(entry->url, url) == 0) {
            if (!entry->is_complete) return NULL;
            if (is_cache_entry_expired(entry)) {
                if (prev) prev->next = entry->next;
                else cache[idx] = entry->next;
                free_cache_entry(entry);
                return NULL;
            }
            return entry->response;
        }
        prev = entry;
        entry = entry->next;
    }
    return NULL;
}

void mark_cache_entry_complete(const char *url) {
    unsigned long idx = hash(url);
    CacheEntry *entry = cache[idx];
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
            CacheEntry *next = entry->next;
            free_cache_entry(entry);
            entry = next;
        }
        cache[i] = NULL;
    }
}
