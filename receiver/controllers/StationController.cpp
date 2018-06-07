#include <unistd.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <arpa/inet.h>
#include <cstring>
#include <thread>
#include <fcntl.h>
#include <boost/log/trivial.hpp>
#include "StationController.h"
#include "MenuController.h"
#include "../../utils/err.h"

void StationController::sendControllPackets() {
    const auto LOOKUP_INTERVAL = std::chrono::seconds(5);
    const auto START_LOOKUP_INTERVAL = std::chrono::milliseconds(500);
    for (int i = 0 ; i < 6 ; i++) {
        ctrlController->sendMessage(LOOKUP);
        std::this_thread::sleep_for(START_LOOKUP_INTERVAL);
    }
    if (!initialized && !senders.empty()) {
        initialized = true;
        currentSender = senders[0];
        dataController->notifyCurrentSenderChange();
        menuController->notifyStateChange();
    }
    std::this_thread::sleep_for(LOOKUP_INTERVAL - START_LOOKUP_INTERVAL);
    while (true) {
        ctrlController->sendMessage(LOOKUP);
        std::this_thread::sleep_for(LOOKUP_INTERVAL);
        bool stateChange = false;
        for (size_t i = 0 ; i < senders.size() ; i++) {
            if (senders[i].secondsFromLastResponse() >= MAX_SECONDS_FROM_LAST_RESPONSE) {
                BOOST_LOG_TRIVIAL(info) << "Sender: " << senders[i] << " not responding, deleting";
                if (senders[i] == currentSender) {
                    if (senders.size() > 1) {
                        currentSender = senders[(i + 1) % senders.size()];
                    } else {
                        initialized = false;
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
    BOOST_LOG_TRIVIAL(info) << "Looking for sender";
    for (Sender &sender : senders) {
        if (reply.getMcast_addr() == sender.getMcastAddr() && reply.getData_port() == sender.getData_port()) {
            BOOST_LOG_TRIVIAL(info) << "Sender " << sender << " already present, updating";
            sender.update();
            return;
        }
    }
    BOOST_LOG_TRIVIAL(info) << "Sender not found";
    addSender(Sender(reply.getName(), reply.getMcast_addr(), reply.getData_port(), ctrl_address));
}

void StationController::addSender(Sender sender) {
    BOOST_LOG_TRIVIAL(info) << "Adding new sender: " << sender;
    //look for a place to add sender to maintain alphabetical order
    bool inserted = false;
    for (size_t i = 0 ; i < senders.size() ; i++) {
        if (senders[i].getName() > sender.getName()) {
            senders.insert(senders.begin() + i, sender);
            inserted = true;
            break;
        }
    }
    if (!inserted) {
        senders.push_back(sender);
    }

    if (!initialized && (firstSender == sender.getName() || firstSender.empty())) {
        currentSender = sender;
        initialized = true;
        dataController->notifyCurrentSenderChange();
    }
    menuController->notifyStateChange();
}


void StationController::nextStation() {
    BOOST_LOG_TRIVIAL(debug) << "Station controller: requested next station";
    if (senders.size() > 1) {
        BOOST_LOG_TRIVIAL(info) << "Current station" << currentSender;
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
    BOOST_LOG_TRIVIAL(info) << "Requested previous station";
    std::lock_guard<std::mutex> lock(sendersMutex);
    if (senders.size() > 1) {
        for (size_t i = 0 ; i < senders.size() ; i++) {
            if (senders[i] == currentSender) {
                currentSender = senders[(i - 1) % senders.size()];
                break;
            }
        }
        menuController->notifyStateChange();
        dataController->notifyCurrentSenderChange();
    }
}
