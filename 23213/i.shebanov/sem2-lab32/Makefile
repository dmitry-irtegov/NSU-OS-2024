CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude
LDFLAGS = -lnsl -lsocket

OBJS = \
    build/client.o \
    build/proxy.o \
    build/request.o \
    build/cachedLoader.o \
    build/nonCachedLoader.o \
    build/loaderProducer.o \
    build/main.o

main: build $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

build:
	mkdir -p build

build/main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o $@

build/client.o: src/client.cpp
	$(CXX) $(CXXFLAGS) -c src/client.cpp -o $@

build/proxy.o: src/proxy.cpp
	$(CXX) $(CXXFLAGS) -c src/proxy.cpp -o $@

build/request.o: src/utils/request.cpp
	$(CXX) $(CXXFLAGS) -c src/utils/request.cpp -o $@

build/cachedLoader.o: src/loader/cachedLoader.cpp
	$(CXX) $(CXXFLAGS) -c src/loader/cachedLoader.cpp -o $@

build/nonCachedLoader.o: src/loader/nonCachedLoader.cpp
	$(CXX) $(CXXFLAGS) -c src/loader/nonCachedLoader.cpp -o $@

build/loaderProducer.o: src/loader/loaderProducer.cpp
	$(CXX) $(CXXFLAGS) -c src/loader/loaderProducer.cpp -o $@
