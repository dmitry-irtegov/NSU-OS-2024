#pragma once

#include<list>
#include<memory>

class Chunk;
class Loader{
public:
    virtual ~Loader() = default;

    virtual void subscribe(int ClientID) = 0;
    virtual void unsubscribe(int ClientID) = 0;
    virtual std::shared_ptr<Chunk> getNextChunk(int ClientID) = 0;
};