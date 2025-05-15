#ifndef CACHE_H
#define CACHE_H

#include <time.h>
#include "network.h"

extern cache* cache_head;

cache* add_to_cache(char* req);
void add_to_data(cache* cac, char* buff, int len);
void fr_data(data* d);
void remove_from_cache(cache* a);
void check_live_time_cache(void);
cache* find_cache(char* buf);

#endif