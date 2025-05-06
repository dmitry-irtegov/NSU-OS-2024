#ifndef UTIL_H
#define UTIL_H

#include "proxy.h"
#include <netdb.h>

int addToPFDs(WorkerState* proxy, int newConn, short event, char type);
void removeFromPFDs(WorkerState* proxy, int newConn);

int addToRequests(WorkerState* proxy, int newConn);
void removeFromRequests(WorkerState* proxy, int newConn);
Request* findRequest(WorkerState* proxy, int indexPFD);

int checkEndOfReq(Buffer* req);
int analyzeRequest(Request* req);

Loader* findLoader(WorkerState* proxy, int indexPFD);

Buffer* initBuffer(int cap, char* source);
void freeBuffer(Buffer* buf);
int resizeBuffer(Buffer* buf);

#endif