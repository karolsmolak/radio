#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <boost/log/trivial.hpp>
#include "CtrlController.h"
#include "../../utils/err.h"

void CtrlController::sendMessage(std::string message, struct sockaddr_in address) {
    if (sendto(ctrlSocket, message.c_str(), message.length(), 0,
               (struct sockaddr *) &address, sizeof address) < 0) {
        syserr("error writing");
    }
}

void CtrlController::sendMessage(std::string message) {
    sendMessage(message, broadcast_address);
}

void CtrlController::readMessages() {
    socklen_t rcva_len;
    ssize_t len;
    struct sockaddr_in sender_ctrl_address;
    char buffer[1024];
    while (true) {
        rcva_len = (socklen_t) sizeof(sender_ctrl_address);
        len = recvfrom(ctrlSocket, buffer, sizeof(buffer), 0,
                       (struct sockaddr *) &sender_ctrl_address, &rcva_len);
        buffer[len] = 0;
        if (len < 0)
            syserr("error on datagram from client socket");
        else if (Reply::isReply(buffer)) {
            Reply reply(buffer);
            stationController->processReply(reply, sender_ctrl_address);
        }
    }
}

CtrlController::CtrlController(std::string &discoverAddr, int ctrlPort) :
        discoverAddr(discoverAddr), ctrlPort(ctrlPort){
    ctrlSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (ctrlSocket < 0) {
        syserr("socket");
    }

    int optval = 1;
    if (setsockopt(ctrlSocket, SOL_SOCKET, SO_BROADCAST, (void*)&optval, sizeof optval) < 0) {
        syserr("setsockopt broadcast");
    }

    optval = 4;
    if (setsockopt(ctrlSocket, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&optval, sizeof optval) < 0) {
        syserr("setsockopt multicast ttl");
    }

    struct sockaddr_in local_address;
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons(0);
    if (bind(ctrlSocket, (struct sockaddr *)&local_address, sizeof local_address) < 0)
        syserr("bind");

    broadcast_address.sin_family = AF_INET;
    broadcast_address.sin_port = htons(ctrlPort);
    if (inet_aton(discoverAddr.c_str(), &broadcast_address.sin_addr) == 0)
        syserr("inet_aton");
}

