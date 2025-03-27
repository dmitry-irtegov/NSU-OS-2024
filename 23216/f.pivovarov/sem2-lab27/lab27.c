#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define BUFFER_LENGTH 255

typedef struct {
    int client;
    int server;
} proxyingPair;
//
int proxyMainLoop(int incomingconnsListener);
int incomingConnsListenerInit(int queueConnectionLength, struct sockaddr_in addr);
int establishNewConnection(int *connectionsCount, int incomingConnsListener);
void fillIntArray(int *arr, int size, int value);
int findFirstCoincedence(int *arr, int size, int value);
int connectToNode();
void closeProxyTunnel(int *descs, int size, proxyingPair *givenProxyingPair);
void removeLiveDesc(int *descs, int size, int desc);
proxyingPair *findPairByDescriptor(int descriptor);
void storePair(proxyingPair *pair);
void removePair(proxyingPair *pair);
void addLiveDesc(int *descs, int size, int desc);
void copySocks(int *arr, int size, fd_set *fdset, int *nfds);
int setupProxy(int portToListen, char *addr, int portToTranslate);

struct sockaddr_in proxyAddr;
struct sockaddr_in nodeAddr;
proxyingPair *proxyingRouter[510] = {0};
int liveReadFDSet[1020];

int main(int argc, char **argv) {
    // Check arguments count
    if (argc != 4) {
        printf(
            "Incorrect arguments.\n\
            1 - port to listen P\n\
            2 - ip for N-node\n\
            3 - port for connection translating P'\n"
        );
        exit(EXIT_FAILURE);
    }
    // Get socket that will listen all incomming connections
    int incomingConnsListener = setupProxy(atoi(argv[1]), argv[2], atoi(argv[3]));
    if (incomingConnsListener == -1) {
        exit(EXIT_FAILURE);
    }

    // Start main loop of proxy logic
    proxyMainLoop(incomingConnsListener);

}

int setupProxy(int portToListen, char *addr, int portToTranslate) {
    char *ip = addr;

    printf("given data: %d %s %d\n", portToListen, ip, portToTranslate);

    // Check numeric arguments correctness
    if (portToListen == 0 || portToTranslate == 0) {
        perror("Incorrect ports\n");
        return -1;
    }

    // Set config for proxyServer address 
    memset(&proxyAddr, 0, sizeof(proxyAddr));
    proxyAddr.sin_family = AF_INET;
    proxyAddr.sin_port = htons(portToListen);
    if (inet_pton(AF_INET, "127.0.0.1", &proxyAddr.sin_addr) != 1) {
        perror("Cannot set the adress");
        return -1;
    }

    // Set config for server 
    memset(&nodeAddr, 0, sizeof(nodeAddr));
    nodeAddr.sin_family = AF_INET;
    nodeAddr.sin_port = htons(portToTranslate);
    nodeAddr.sin_addr.s_addr = inet_addr(ip);

    // init socket to listen clients connections
    int incomingConnsListener = incomingConnsListenerInit(510, proxyAddr);

    return incomingConnsListener;
}

int proxyMainLoop(int incomingConnsListener) {
    // Work with connections
    int connectionsCount = 0;
    fillIntArray(liveReadFDSet, 1020, -1);
    fd_set readfds;
    
    proxyingPair *foundedProxyingPair;

    int readedBytes, wrotedBytes;
    int fdNum;

    char buffer[BUFFER_LENGTH];

    int nfds;
    printf("Start waiting for incomming connections!\n");
    while (1) {
        nfds = incomingConnsListener;
        FD_ZERO(&readfds);
        FD_SET(incomingConnsListener, &readfds);

        copySocks(liveReadFDSet, 510, &readfds, &nfds);
        nfds++;
        
        fdNum = select(nfds, &readfds, NULL, NULL, NULL);
        printf("nfds: %d || FDNUM: %d\n", nfds, fdNum);
        if (fdNum == -1) {
            perror("Something with select");
            exit(EXIT_FAILURE);
        }
        
        if (FD_ISSET(incomingConnsListener, &readfds)) {
            switch (establishNewConnection(&connectionsCount, incomingConnsListener)) {
                case -1:
                    return -1;
                case 0:
                    fdNum--;
            }
        }
        for (int i = 0; i < 1020; i++) {
            if (FD_ISSET(liveReadFDSet[i], &readfds) == 0) {
                continue;
            }
            printf("Data transmission\n");
            // try to find pair where is dscriptor containing
            foundedProxyingPair = findPairByDescriptor(liveReadFDSet[i]);
            if (foundedProxyingPair == NULL) {
                perror("This descriptor does not contains in any pair");
                return -1;
            }
            // Data transmission
            readedBytes = recv(liveReadFDSet[i], buffer, BUFFER_LENGTH, 0);
            printf("Given data: %s\n", buffer);
            switch (readedBytes) {
                case -1:
                    perror("Something went wrong");
                    return -1;
                case 0:
                    printf("Connection closed!\n");
                    FD_CLR(foundedProxyingPair->client, &readfds);
                    FD_CLR(foundedProxyingPair->server, &readfds);
                    closeProxyTunnel(liveReadFDSet, 1020, foundedProxyingPair);
                    connectionsCount--;
                    break;
                default:
                    wrotedBytes = send(
                        foundedProxyingPair->client == liveReadFDSet[i] 
                            ? foundedProxyingPair->server 
                            : foundedProxyingPair->client,
                        buffer,
                        readedBytes,
                        0
                    );
            }
                
            if (wrotedBytes < 0) {
                perror("Can't send data from this.");
                return -1;
                break;
            }
        }       
    }

    return 0;
}

int establishNewConnection(int *connectionsCount, int incomingConnsListener) {
    // Detect incomming connection
    printf("Detect incomming connection\n");

    int addressLen = sizeof(proxyAddr);
    // Get new connection
    int newClient = accept(incomingConnsListener, (struct sockaddr *)&proxyAddr, (socklen_t *)&addressLen);
    if (newClient == -1) {
        perror("newSocket is -1");
        return -1;
    }
    if (*connectionsCount >= 510) {
        printf("Too much connections exitsts.\n");
        close(newClient);
    } else {
        int newNode = connectToNode();
        if (newNode == -1) {
            perror("Cannot establish new connection with node");
            return 1;
        }

        proxyingPair *newProxyingPair = (proxyingPair *)malloc(sizeof(proxyingPair));
        if (newProxyingPair == NULL) {
            perror("cannot cannot allocate memory to new proxying pair");
            return -1;
        }
        newProxyingPair->client = newClient;
        newProxyingPair->server = newNode;

        storePair(newProxyingPair);
        
        addLiveDesc(liveReadFDSet, 1020, newClient);
        addLiveDesc(liveReadFDSet, 1020, newNode);

        connectionsCount++;
    }

    return 0;
}
int incomingConnsListenerInit(int queueConnectionLength, struct sockaddr_in addr) {
    int socketDescriptor = socket(AF_INET, SOCK_STREAM, 6);
    if (socketDescriptor == -1) {
        perror("Cannot create socketFD");
        return -1;
    }

    if (bind(socketDescriptor, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Cannot bind");
        close(socketDescriptor);
        return -1;
    }

    if (listen(socketDescriptor, queueConnectionLength) == -1) {
        perror("Cannot listen");
        close(socketDescriptor);
        return -1;
    }

    return socketDescriptor;
}



proxyingPair *findPairByDescriptor(int descriptor) {
    for (int i = 0; i < 510; i++) 
        if (proxyingRouter[i]->client == descriptor || proxyingRouter[i]->server == descriptor) 
            return proxyingRouter[i];
        
    return NULL;
}

int connectToNode() {
    int nodeFD = socket(AF_INET, SOCK_STREAM, 0);
    if (nodeFD == -1) {
        perror("somthing with nodeFD");
        return -1;
    }

    printf("nodeFD: %d || sizeofNodeAddr: %ld\n", nodeFD, sizeof(nodeAddr));
    if (connect(nodeFD, (struct sockaddr*)&nodeAddr, sizeof(nodeAddr)) == -1) {
        perror("Something with connect()");
        return -1;
    }

    return nodeFD;
}

void storePair(proxyingPair *pair) {
    for (int i = 0; i < 510; i++)
        if (proxyingRouter[i] == NULL) {
            proxyingRouter[i] = pair;
            return;
        }
}

void removePair(proxyingPair *pair) {
    for (int i = 0; i < 510; i++)
        if (proxyingRouter[i] == pair) {
            proxyingRouter[i] = NULL;
            return;
        }
}

void closeProxyTunnel(int *descs, int size, proxyingPair *givenProxyingPair) {
    close(givenProxyingPair->client);
    close(givenProxyingPair->server);
    removePair(givenProxyingPair);
    printf("Try delete client\n");
    removeLiveDesc(descs, size, givenProxyingPair->client);
    printf("removing server\n");
    removeLiveDesc(descs, size, givenProxyingPair->server);
    free(givenProxyingPair);
}

void fillIntArray(int *arr, int size, int value) {
    for (int i = 0; i < size; i++) 
        arr[i] = value;
}

int findFirstCoincedence(int *arr, int size, int value) {
    for (int i = 0; i < size; i++)
        if (arr[i] == value)
            return i;

    return -1;
}

void copySocks(int *arr, int size, fd_set *fdset, int *nfds) {
    printf("COPYING SOCKS\n");
    for (int i = 0; i < size; i++) { 
        if (arr[i] > -1) {
            printf("set _ index %d || desc %d\n", i, arr[i]);
            FD_SET(arr[i], fdset);
        }
        if (arr[i] > *nfds) {
            *nfds = arr[i];
        }
    }
}

void addLiveDesc(int *descs, int size, int desc) {
    for (int i = 0; i < size; i++) {
        if (descs[i] == -1) {
            descs[i] = desc;
            return;
        }
    }
}

void removeLiveDesc(int *descs, int size, int desc) {
    for (int i = 0; i < size; i++) {
        if (descs[i] == desc) {
            printf("del _ index %d || desc %d\n", i, descs[i]);
            descs[i] = -1;
            return;
        }
    }
    printf("Cannot find %d\n", desc);
}
