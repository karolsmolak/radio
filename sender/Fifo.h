#ifndef SIKRADIO_FIFO_H
#define SIKRADIO_FIFO_H


#include <string>
#include <cstring>
#include <boost/log/trivial.hpp>
#include <mutex>

class Fifo {
    int fsize;
    unsigned char *data;
    int end = 0;
    uint64_t bytesReceived = 0;
    std::mutex fifoMutex;
public:
    Fifo(int fsize) : fsize(fsize) {
        data = new unsigned char[fsize];
    }

    bool getBytes(uint64_t firstByte, int psize, unsigned char *dest);
    void newByte(unsigned char byte);

    ~Fifo() {
        delete[] data;
    }
};


#endif //SIKRADIO_FIFO_H
