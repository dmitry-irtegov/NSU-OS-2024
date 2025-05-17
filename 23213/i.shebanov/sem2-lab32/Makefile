build/main: build/main.o build/src/client.o build/src/proxy.o build/src/utils/request.o build/src/loader/cachedLoader.o build/src/loader/nonCachedLoader.o build/src/loader/loaderProducer.o
	g++ -Wall -lnsl -lsocket build/main.o build/src/client.o build/src/proxy.o build/src/utils/request.o build/src/loader/cachedLoader.o build/src/loader/nonCachedLoader.o build/src/loader/loaderProducer.o -o build/main

build/main.o: main.cpp
	mkdir -p build
	g++ -Wall -Iinclude -std=c++17 -c main.cpp -o build/main.o

build/src/client.o: src/client.cpp
	mkdir -p build/src
	g++ -Wall -Iinclude -std=c++17 -c src/client.cpp -o build/src/client.o

build/src/proxy.o: src/proxy.cpp
	mkdir -p build/src
	g++ -Wall -Iinclude -std=c++17 -c src/proxy.cpp -o build/src/proxy.o

build/src/utils/request.o: src/utils/request.cpp
	mkdir -p build/src/utils
	g++ -Wall -Iinclude -std=c++17 -c src/utils/request.cpp -o build/src/utils/request.o

build/src/loader/cachedLoader.o: src/loader/cachedLoader.cpp
	mkdir -p build/src/loader
	g++ -Wall -Iinclude -std=c++17 -c src/loader/cachedLoader.cpp -o build/src/loader/cachedLoader.o

build/src/loader/nonCachedLoader.o: src/loader/nonCachedLoader.cpp
	mkdir -p build/src/loader
	g++ -Wall -Iinclude -std=c++17 -c src/loader/nonCachedLoader.cpp -o build/src/loader/nonCachedLoader.o

build/src/loader/loaderProducer.o: src/loader/loaderProducer.cpp
	mkdir -p build/src/loader
	g++ -Wall -Iinclude -std=c++17 -c src/loader/loaderProducer.cpp -o build/src/loader/loaderProducer.o
