#ifndef UTIL_H
#define UTIL_H

#include "proxy.h"
#include <netdb.h>

extern ProxyState proxy;

int addToPFDs(int newConn, short event, char type);
void removeFromPFDs(int newConn);

int addToRequests(int newConn);
void removeFromRequests(int newConn);
Request* findRequest(int indexPFD);

int checkEndOfReq(Buffer* req);
int analyzeRequest(Request* req);

Loader* findLoader(int indexPFD);

Buffer* initBuffer(int cap, char* source);
void freeBuffer(Buffer* buf);
int resizeBuffer(Buffer* buf);

#endif