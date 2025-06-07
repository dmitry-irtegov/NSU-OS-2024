CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude -pthread
LDFLAGS = -lnsl -lsocket -pthread

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

MAINDEPS = main.cpp include/proxy.hpp
build/main.o: $(MAINDEPS)
	$(CXX) $(CXXFLAGS) -c main.cpp -o $@

CLIENTDEPS = \
    src/client.cpp \
    include/client.hpp \
    include/utils/request.hpp \
    include/chunk.hpp \
    include/loader/loader.hpp \
    include/loader/loaderProducer.hpp
build/client.o: $(CLIENTDEPS)
	$(CXX) $(CXXFLAGS) -c src/client.cpp -o $@

PROXYDEPS = \
    src/proxy.cpp \
    include/proxy.hpp \
    include/client.hpp
build/proxy.o: $(PROXYDEPS)
	$(CXX) $(CXXFLAGS) -c src/proxy.cpp -o $@

REQUESTDEPS = \
    src/utils/request.cpp \
    include/utils/request.hpp
build/request.o: $(REQUESTDEPS)
	$(CXX) $(CXXFLAGS) -c src/utils/request.cpp -o $@

CACHEDLOADERDEPS = \
    src/loader/cachedLoader.cpp \
    include/loader/cachedLoader.hpp \
    include/loader/loaderProducer.hpp \
    include/utils/appendOnlyLog.hpp \
    include/chunk.hpp
build/cachedLoader.o: $(CACHEDLOADERDEPS)
	$(CXX) $(CXXFLAGS) -c src/loader/cachedLoader.cpp -o $@

NONCACHEDLOADERDEPS = \
	src/loader/nonCachedLoader.cpp \
    include/loader/nonCachedLoader.hpp \
    include/chunk.hpp \
    include/utils/request.hpp
build/nonCachedLoader.o: $(NONCACHEDLOADERDEPS)
	$(CXX) $(CXXFLAGS) -c src/loader/nonCachedLoader.cpp -o $@

LOADERPRODUCERDEPS = \
    include/loader/cachedLoader.hpp \
    include/loader/loaderProducer.hpp \
    include/loader/nonCachedLoader.hpp
build/loaderProducer.o: $(LOADERPRODUCERDEPS)
	$(CXX) $(CXXFLAGS) -c src/loader/loaderProducer.cpp -o $@
