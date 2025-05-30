#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cache.h"

cache* cache_head = NULL;

void add_to_data(cache* cac, char* buff, int len) {
    if (!cac || !buff || len <= 0) return;

    data* new_node = (data*)malloc(sizeof(data));
    if (!new_node) return;

    new_node->data = (char*)malloc(len);
    if (!new_node->data) {
        free(new_node);
        return;
    }

    memcpy(new_node->data, buff, len);
    new_node->len = len;
    new_node->next = NULL;

    if (!cac->dat) {
        cac->dat = new_node;
    }
    else {
        data* curr = cac->dat;
        while (curr->next) {
            curr = curr->next;
        }
        curr->next = new_node;
    }
}

cache* add_to_cache(char* req) {

    cache* new_cache = (cache*)malloc(sizeof(cache));
    if (!new_cache) return NULL;

    new_cache->request = strdup(req);
    new_cache->live_time = -1;
    new_cache->birth_time = time(NULL);
    new_cache->working = 0;
    new_cache->dat = NULL;
    new_cache->next = NULL;
    new_cache->status_code = -1;
    cache* t;
    if (!cache_head) {
        cache_head = new_cache;
        printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAaa\n");
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
        if (!cur->working && !strcmp(buf, cur->request)) return cur;
        cur = cur->next;
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
        if ((now - cur->birth_time) > cur->live_time && !cur->working) {
            printf("\nCleared cache:\n==============================================\n%s\n==============================================\n", cur->request);
            remove_from_cache(cur);
        }
        cur = next;
    }
}