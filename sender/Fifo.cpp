#include "Fifo.h"

bool Fifo::getBytes(uint64_t firstByte, int psize, uint8_t *dest) {
    std::lock_guard<std::mutex> lock(fifoMutex);
    if (bytesReceived >= firstByte + psize && firstByte + fsize >= bytesReceived) {
        int begin = firstByte % fsize;
        for (int i = begin ; i < begin + psize ; i++) {
            dest[i - begin] = data[i % fsize];
        }
        return true;
    } else {
        return false;
    }
}

void Fifo::newByte(uint8_t byte) {
    std::lock_guard<std::mutex> lock(fifoMutex);
    data[end] = byte;
    end = (end + 1) % fsize;
    bytesReceived++;
}
