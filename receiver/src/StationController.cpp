#include <unistd.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <arpa/inet.h>
#include <cstring>
#include <thread>
#include <fcntl.h>
#include <iostream>
#include "../include/StationController.h"
#include "../include/MenuController.h"
#include "../../utils/err.h"

void StationController::sendControllPackets() {
    const auto LOOKUP_INTERVAL = std::chrono::seconds(5);
    const auto START_LOOKUP_INTERVAL = std::chrono::milliseconds(500);
    sendLookupsOnStart(START_LOOKUP_INTERVAL);
    sendersMutex.lock();
    if (!initialized && hasSenders()) {
        initialized = true;
        currentSender = senders[0];
        dataController->notifyCurrentSenderChange();
        menuController->notifyStateChange();
    }
    sendersMutex.unlock();
    std::this_thread::sleep_for(LOOKUP_INTERVAL - START_LOOKUP_INTERVAL);
    while (true) {
        ctrlController->sendMessage(LOOKUP);
        std::this_thread::sleep_for(LOOKUP_INTERVAL);
        bool stateChange = false;
        std::lock_guard<std::mutex> lock(sendersMutex);
        for (int i = 0 ; i < senders.size() ; i++) {
            if (senders[i].secondsFromLastResponse() >= MAX_SECONDS_FROM_LAST_RESPONSE) {
                if (senders[i] == currentSender) {
                    if (isSenderChangeable()) {
                        currentSender = senders[(i + 1) % senders.size()];
                    }
                    dataController->notifyCurrentSenderChange();
                }
                senders.erase(senders.begin() + i);
                stateChange = true;
                i--;
            }
        }
        if (stateChange) {
            menuController->notifyStateChange();
        }
    }
}

void StationController::processReply(Reply reply, struct sockaddr_in ctrl_address) {
    std::lock_guard<std::mutex> lock(sendersMutex);
    for (Sender &sender : senders) {
        if (reply.getMcast_addr() == sender.getMcastAddr() && reply.getData_port() == sender.getData_port()) {
            sender.update();
            return;
        }
    }
    addSender(Sender(reply.getName(), reply.getMcast_addr(), reply.getData_port(), ctrl_address));
}

void StationController::addSender(Sender sender) {
    //look for a place to add sender to maintain alphabetical order
    bool inserted = false;
    for (size_t i = 0 ; i < senders.size() ; i++) {
        if (senders[i] > sender) {
            senders.insert(senders.begin() + i, sender);
            inserted = true;
            break;
        }
    }
    if (!inserted) {
        senders.push_back(sender);
    }

    if ((!initialized && (firstSender == sender.getName() || firstSender.empty())) || (initialized && !hasSenders())) {
        currentSender = sender;
        initialized = true;
        dataController->notifyCurrentSenderChange();
    }
    menuController->notifyStateChange();
}

void StationController::nextStation() {
    std::lock_guard<std::mutex> lock(sendersMutex);
    if (isSenderChangeable()) {
        for (size_t i = 0 ; i < senders.size() ; i++) {
            if (senders[i] == currentSender) {
                currentSender = senders[(i + 1) % senders.size()];
                break;
            }
        }
        dataController->notifyCurrentSenderChange();
        menuController->notifyStateChange();
    }
}

void StationController::previousStation() {
    std::lock_guard<std::mutex> lock(sendersMutex);
    if (isSenderChangeable()) {
        for (size_t i = 0 ; i < senders.size() ; i++) {
            if (senders[i] == currentSender) {
                currentSender = senders[(i + senders.size() - 1) % senders.size()];
                break;
            }
        }
        dataController->notifyCurrentSenderChange();
        menuController->notifyStateChange();
    }
}


void StationController::sendLookupsOnStart(const std::chrono::duration<int64_t, std::milli> &START_LOOKUP_INTERVAL) const {
    for (int i = 0 ; i < START_LOOKUP_RETRIES; i++) {
        ctrlController->sendMessage(LOOKUP);
        std::this_thread::sleep_for(START_LOOKUP_INTERVAL);
    }
}

bool StationController::isSenderChangeable() const {
    return senders.size() > 1;
}
