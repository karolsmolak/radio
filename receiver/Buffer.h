#ifndef SIKRADIO_BUFFER_H
#define SIKRADIO_BUFFER_H


#include <boost/log/trivial.hpp>
#include <mutex>
#include <thread>
#include <iostream>
#include "../messages/messages.h"

//Class responsible for storing audio data
class Buffer {
    int size;
    int psize;
    uint64_t begin = 0;
    uint8_t *buffer;
    uint64_t maxReceived;
    uint64_t byte0;
    bool *filled;
    std::atomic_bool flushing = false;
    bool lackingBytes = false;
    std::mutex bufferLock;
public:

    Buffer(int size) : size(size) {
        buffer = new uint8_t[size];
        filled = new bool[size];
    }

    void clear(uint64_t byte0, int psize) {
        flushing = false;
        lackingBytes = false;
        for (int i = 0 ; i < size ; i++) {
            filled[i] = false;
        }
        this->byte0 = byte0;
        begin = byte0;
        this->psize = psize;
        this->maxReceived = byte0;
    }

    bool ready() {
        std::lock_guard<std::mutex> lock(bufferLock);
        return maxReceived + psize > byte0 + 3 * size / 4;
    }

    void storePackage(Package &package);

    bool printNextSample();

    void flush();

    void setFlushing(bool flushing) {
        Buffer::flushing = flushing;
    }

    bool isFlushing() const {
        return flushing;
    }

    ~Buffer() {
        delete[] buffer;
        delete[] filled;
    }

    bool hasLackingBytes() {
        return lackingBytes;
    }
};


#endif //SIKRADIO_BUFFER_H
