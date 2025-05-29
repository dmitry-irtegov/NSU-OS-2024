#pragma once

#include <cstring>
#include <cstddef>
#include <stdexcept>

constexpr ssize_t CHUNK_CAPACITY = 1024;

class Chunk {
public:
    Chunk(const char* data, ssize_t size, bool isFinal, bool isError) : isFinal(isFinal), isError(isError), size_(size) {
        if (size > CHUNK_CAPACITY) {
            throw std::runtime_error("Chunk size exceeds capacity");
        }
        if(data == nullptr) {
            return;
        }
        if(size < 0) {
            size = 0;
        }
        std::memcpy(data_, data, size);
    }

    const char* data() { return data_; }
    size_t size() { return size_; }
    bool isFinal;
    bool isError;

private:
    char data_[CHUNK_CAPACITY];
    size_t size_;
};
