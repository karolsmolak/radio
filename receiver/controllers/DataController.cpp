#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <boost/log/trivial.hpp>
#include <poll.h>
#include "DataController.h"
#include "../../utils/err.h"
#include "../../messages/messages.h"

void DataController::play() {
    int dataSocket = sender.getDataSocket();
    bool first = true;
    uint64_t session_id;
    uint64_t byte0;
    int psize;
    int rcv_len;
    Package package;
    std::thread out;
    while (isPlaying) {
        rcv_len = (int) read(dataSocket, &package, 1500);
        if (rcv_len > 0) {
            if (first || package.session_id > session_id || buffer->hasLackingBytes()) {
                if (out.joinable()) {
                    buffer->setFlushing(false);
                    out.join();
                }
                session_id = package.session_id;
                byte0 = package.first_byte_num;
                psize = rcv_len - 16;
                buffer->clear(byte0, psize);
                retransmissionController->restart(byte0, psize);
            }
            if (package.session_id == session_id) {
                buffer->storePackage(package);
                if (buffer->ready() && !buffer->isFlushing()) {
                    buffer->setFlushing(true);
                    out = std::thread(&Buffer::flush, buffer);
                }
                retransmissionController->newPackage(package.first_byte_num);
            }
            first = false;
        }
    }
    if (out.joinable()) {
        buffer->setFlushing(false);
        out.join();
    }
}

void DataController::notifyCurrentSenderChange() {
    if (isPlaying) {
        sender.closeDataSocket();
        isPlaying = false;
        playThread.join();
    }
    if (stationController->hasSenders()) {
        sender = stationController->getCurrentSender();
        sender.initializeDataSocket();
        isPlaying = true;
        playThread = std::thread(&DataController::play, this);
    }
}
