#ifndef UTIL_H
#define UTIL_H

#include "proxy.h"
#include <netdb.h>

int addToPFDs(int newConn, short event, char type);

int addToRequests(int newConn);
Request* findRequest(int indexPFD);
int checkEndOfReq(Buffer* req);

Loader* findLoader(int indexPFD);

Buffer* initBuffer(int cap, char* source);
void freeBuffer(Buffer* buf);
int resizeBuffer(Buffer* buf);

#endif