CXX := g++
CXXFLAGS := -Wall -std=c++17 -Iinclude
LDFLAGS := -lnsl -lsocket

BUILD_DIR := build
SRC_DIR := src

OBJS := \
	$(BUILD_DIR)/main.o \
	$(BUILD_DIR)/client.o \
	$(BUILD_DIR)/proxy.o \
	$(BUILD_DIR)/request.o \
	$(BUILD_DIR)/cachedLoader.o \
	$(BUILD_DIR)/nonCachedLoader.o \
	$(BUILD_DIR)/loaderProducer.o

main: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

$(BUILD_DIR)/main.o: main.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/client.o: $(SRC_DIR)/client.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/proxy.o: $(SRC_DIR)/proxy.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/request.o: $(SRC_DIR)/utils/request.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/cachedLoader.o: $(SRC_DIR)/loader/cachedLoader.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/nonCachedLoader.o: $(SRC_DIR)/loader/nonCachedLoader.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/loaderProducer.o: $(SRC_DIR)/loader/loaderProducer.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
