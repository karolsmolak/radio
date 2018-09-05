#include <iostream>
#include "../include/Buffer.h"

void Buffer::storePackage(Package &package) {
    std::lock_guard<std::mutex> lock(bufferLock);
    uint64_t firstByte = package.first_byte_num;
    if (maxReceived + psize >= size + firstByte) {
        return; //package too old
    }
    if (firstByte > maxReceived) {
        for (uint64_t i = firstByte - 1 ; i >= maxReceived + psize ; i--) {
            filled[i % size] = false;
        }
        maxReceived = firstByte;
    }
    int pos = firstByte % (uint64_t)size;
    for (int i = 0 ; i < psize ; i++) {
        buffer[(pos + i) % size] = package.audio_data[i];
        filled[(pos + i) % size] = true;
    }
}

bool Buffer::printNextSample() {
    std::lock_guard<std::mutex> lock(bufferLock);
    if (begin + size < maxReceived + psize) {
        return false;
    } else if (begin + 3 >= maxReceived + psize) {
        return false;
    }
    for (int i = 0 ; i < 4 ; i++) {
        if (!filled[(begin + i) % size]) {
            return false;
        }
    }
    for (int i = 0 ; i < 4 ; i++) {
        putchar(buffer[begin % size]);
        filled[begin % size] = false;
        begin++;
    }
    return true;
}

void Buffer::flush() {
    while (flushing) {
        if (!printNextSample()) {
            lackingBytes = true;
            return;
        }
    }
}
