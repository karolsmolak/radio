#include "Fifo.h"

bool Fifo::getBytes(uint64_t firstByte, int psize, unsigned char *dest) {
    std::lock_guard<std::mutex> lock(fifoMutex);
    if (bytesReceived >= firstByte + psize && firstByte >= bytesReceived - fsize) {
        int begin = firstByte % fsize;
        for (int i = begin ; i < begin + psize ; i++) {
            dest[i - begin] = data[i % fsize];
        }
        return true;
    } else {
        return false;
    }
}

void Fifo::newByte(unsigned char byte) {
    std::lock_guard<std::mutex> lock(fifoMutex);
    data[end] = byte;
    end = (end + 1) % fsize;
    bytesReceived++;
}
