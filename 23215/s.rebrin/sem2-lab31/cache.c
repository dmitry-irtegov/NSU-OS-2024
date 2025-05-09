#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cache.h"

cache* cache_head = NULL;

void add_to_data(cache* cac, char* buff, int len) {

    cac->dat = (data*)malloc(sizeof(data));

    cac->dat->len = len;
    cac->dat->data = (char*)malloc(len);
    if (!cac->dat->data) {
        free(cac->dat);
        free(cac->request);
        free(cac);
        return NULL;
    }
    memcpy(cac->dat->data, buff, len);
    cac->dat->next = NULL;
}

cache* add_to_cache(char* req, int live) {

    cache* new_cache = (cache*)malloc(sizeof(cache));
    if (!new_cache) return NULL;

    new_cache->request = strdup(req);
    new_cache->live_time = live;
    new_cache->birth_time = time(NULL);
    new_cache->working = 0;
    new_cache->dat = NULL;
    new_cache->next = NULL;

    if (!cache_head) {
        cache_head = new_cache;
    }
    else {
        t = cache_head;
        while (t->next) t = t->next;
        t->next = new_cache;
    }

    return new_cache;
}

void fr_data(data* d) {
    while (d) {
        data* next = d->next;
        free(d->data);
        free(d);
        d = next;
    }
}

cache* find_cache(char* buf) {
    cache* cur = cache_head;
    while (cur) {
        if (!strcmp(buf, cur->request)) return cur;
    }
    return NULL;
}

void remove_from_cache(cache* a) {
    if (!a) return;

    cache** p = &cache_head;
    while (*p && *p != a) p = &(*p)->next;
    if (*p) {
        *p = a->next;
        fr_data(a->dat);
        free(a->request);
        free(a);
    }
}

void check_live_time_cache() {
    cache* cur = cache_head;
    time_t now = time(NULL);
    while (cur) {
        cache* next = cur->next;
        if (cur->live_time > 0 && (now - cur->birth_time) > cur->live_time && !cur->working) {
            printf("\nCleared cache:\n==============================================\n%s\n==============================================\n", cur->request);
            remove_from_cache(cur);
        }
        cur = next;
    }
}