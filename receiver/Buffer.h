#ifndef SIKRADIO_BUFFER_H
#define SIKRADIO_BUFFER_H


#include <boost/log/trivial.hpp>
#include <mutex>
#include <thread>
#include "../messages/messages.h"

//Class responsible for storing audio data
class Buffer {
    int size;
    int psize;
    uint64_t begin = 0;
    unsigned char *buffer;
    uint64_t maxReceived;
    uint64_t byte0;
    bool *filled;
    std::atomic_bool flushing = false;
    bool lackingBytes = false;
    std::mutex bufferLock;
    unsigned char getNextByte();
public:

    Buffer(int size) : size(size) {
        buffer = new unsigned char[size];
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
        return maxReceived + psize > byte0 + 3 * size / 4;
    }
    void storePackage(Package &package);

    bool hasNextSample() {
        std::lock_guard<std::mutex> lock(bufferLock);
        if (begin < maxReceived + psize - size) {
            return false;
        } else if (begin + 3 >= maxReceived + psize) {
            return false;
        }
        for (int i = 0 ; i < 4 ; i++) {
            if (!filled[(begin + i) % size]) {
                return false;
            }
        }
        return true;
    }

    void flush() {
        while (flushing) {
            if (hasNextSample()) {
                putchar(getNextByte());
                putchar(getNextByte());
                putchar(getNextByte());
                putchar(getNextByte());
            } else {
                lackingBytes = true;
                return;
            }
        }
    }

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
