//
// Created by karol on 24.05.18.
//

#ifndef SIKRADIO_BUFFER_H
#define SIKRADIO_BUFFER_H


#include <boost/log/trivial.hpp>
#include <mutex>
#include <thread>
#include "../messages/messages.h"

//Class responsible for storing audio data
class Buffer {
    int size;
    int begin = 0;
    unsigned char *buffer;
    uint64_t maxReceived;
    uint64_t byte0;
    bool *filled;
    bool flushing = false;
    bool lackingBytes = false;
    std::mutex bufferLock;
public:
    Buffer(int size) : size(size) {
        buffer = new unsigned char[size];
        filled = new bool[size];
    }

    void clear(uint64_t byte0) {
        flushing = false;
        lackingBytes = false;
        for (int i = 0 ; i < size ; i++) {
            filled[i] = false;
        }
        this->byte0 = byte0;
        begin = byte0 % size;
        this->maxReceived = byte0;
    }

    bool ready() {
        return maxReceived > byte0 + 3 * size / 4;
    }

    void storePackage(Package &package, int psize);
    unsigned char getNextByte();

    bool hasNextByte() {
        return filled[begin];
    }

    void flush() {
        int bytes = 0;
        while (flushing) {
            if (hasNextByte()) {
                putchar(getNextByte());
                bytes++;
            } else {
                lackingBytes = true;
                BOOST_LOG_TRIVIAL(info) << "No bytes, finishing ";
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
        delete buffer;
        delete filled;
    }

    bool hasLackingBytes() {
        return lackingBytes;
    }
};


#endif //SIKRADIO_BUFFER_H
