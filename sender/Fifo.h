#ifndef SIKRADIO_FIFO_H
#define SIKRADIO_FIFO_H


#include <string>
#include <cstring>
#include <boost/log/trivial.hpp>
#include <mutex>

class Fifo {
    int fsize;
    uint8_t *data;
    int end = 0;
    uint64_t bytesReceived = 0;
    std::mutex fifoMutex;
public:
    Fifo(int fsize) : fsize(fsize) {
        data = new uint8_t[fsize];
    }

    bool getBytes(uint64_t firstByte, int psize, uint8_t *dest);
    void newByte(uint8_t byte);

    ~Fifo() {
        delete[] data;
    }
};


#endif //SIKRADIO_FIFO_H
