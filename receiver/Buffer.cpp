#include "Buffer.h"

void Buffer::storePackage(Package &package) {
    std::lock_guard<std::mutex> lock(bufferLock);
    if (maxReceived + psize - package.first_byte_num >= (uint64_t)size) {
        return; //package too old
    }
    if (package.first_byte_num > maxReceived) {
        for (uint64_t i = package.first_byte_num - 1 ; i >= maxReceived + psize ; i--) {
            filled[i % size] = false;
        }
        maxReceived = package.first_byte_num;
    }
    int pos = (package.first_byte_num - byte0) % (uint64_t)size;
    for (int i = 0 ; i < psize ; i++) {
        buffer[(pos + i) % size] = package.audio_data[i];
        filled[(pos + i) % size] = true;
    }
}

unsigned char Buffer::getNextByte() {
    std::lock_guard<std::mutex> lock(bufferLock);
    unsigned char result = buffer[begin % size];
    filled[begin % size] = false;
    begin++;
    return result;
}