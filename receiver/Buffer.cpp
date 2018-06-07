#include "Buffer.h"

void Buffer::storePackage(Package &package, int psize) {
    std::lock_guard<std::mutex> lock(bufferLock);
    int pos = (package.first_byte_num - byte0) % size;
    for (int i = 0 ; i < psize ; i++) {
        buffer[(pos + i) % size] = package.audio_data[i];
        filled[(pos + i) % size] = true;
    }
    for (uint64_t i = package.first_byte_num - 1 ; i >= maxReceived + psize ; i--) {
        filled[i % size] = false;
    }
    if (package.first_byte_num > maxReceived) {
        maxReceived = package.first_byte_num;
    }
}

unsigned char Buffer::getNextByte() {
    std::lock_guard<std::mutex> lock(bufferLock);
    unsigned char result = buffer[begin];
    filled[begin] = false;
    begin = (begin + 1) % size;
    return result;
}