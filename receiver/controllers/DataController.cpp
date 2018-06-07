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
    BOOST_LOG_TRIVIAL(info) << "Playing station " << sender;
    int dataSocket = sender.getDataSocket();
    bool first = true;
    uint64_t session_id;
    uint64_t byte0;
    int psize;
    int rcv_len;
    Package package;
    std::thread out;
    while (isPlaying) {
        rcv_len = read(dataSocket, &package, 1000);
        if (rcv_len < 0) {
            BOOST_LOG_TRIVIAL(info) << "Not getting data";
        } else {
            if (first || package.session_id > session_id || buffer->hasLackingBytes()) {
                if (out.joinable()) {
                    out.join();
                }
                session_id = package.session_id;
                byte0 = package.first_byte_num;
                psize = rcv_len - 16;
                buffer->clear(byte0);
                retransmissionController->restart(byte0, psize);
            }
            if (package.session_id == session_id) {
                buffer->storePackage(package, psize);
                if (buffer->ready() && !buffer->isFlushing()) {
                    BOOST_LOG_TRIVIAL(info) << "Buffer filled, printing";
                    buffer->setFlushing(true);
                    out = std::thread(&Buffer::flush, buffer);
                }
                retransmissionController->newPackage(package.first_byte_num);
            }
            first = false;
        }
    }
    if (out.joinable()) {
        out.join();
    }
}

void DataController::notifyCurrentSenderChange() {
    BOOST_LOG_TRIVIAL(info) << "DATA CONTROLLER: sender changed";
    if (isPlaying) {
        BOOST_LOG_TRIVIAL(info) << "Closing socket";
        sender.closeDataSocket();
        buffer->setFlushing(false);
        isPlaying = false;
        playThread.join();
    }
    if (stationController->isInitialized()) {
        sender = stationController->getCurrentSender();
        sender.initializeDataSocket();
        isPlaying = true;
        playThread = std::thread(&DataController::play, this);
    }
}
