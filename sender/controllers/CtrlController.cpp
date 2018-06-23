#include "CtrlController.h"

void CtrlController::readMessages() {
    socklen_t snda_len, rcva_len;
    ssize_t len, snd_len;
    struct sockaddr_in client_address;
    char buffer[1024];
    while (!finished) {
        rcva_len = (socklen_t) sizeof(client_address);
        len = recvfrom(ctrlSocket, buffer, sizeof(buffer), 0,
                           (struct sockaddr *) &client_address, &rcva_len);
        if (len == -1) {
            continue;
        }
        if (len < 0)
            syserr("error on datagram from client socket");
        else {
            buffer[len] = 0;
            snda_len = (socklen_t) sizeof(client_address);
            if (Rexmit::isRexmit(buffer)) {
                Rexmit rexmit(buffer);
                retransmissionController->retransmit(rexmit);
            } else if (!strcmp(buffer, LOOKUP.c_str())) {
                Reply reply(mcastAddr, dataPort, name);
                std::string message = reply.toString();
                snd_len = sendto(ctrlSocket, message.c_str(), message.length(), 0,
                                 (struct sockaddr *) &client_address, snda_len);
                if (snd_len != message.length())
                    syserr("error on sending datagram to client socket");
            }
        }
    }
}

CtrlController::CtrlController(int ctrlPort, int dataPort, std::string name, std::string mcastAddr) : port(ctrlPort), dataPort(dataPort), name(name), mcastAddr(mcastAddr) {
    ctrlSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (ctrlSocket < 0)
        syserr("socket");

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if (setsockopt(ctrlSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Error");
    }

    struct sockaddr_in local_address;
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons(port);
    if (bind(ctrlSocket, (struct sockaddr *)&local_address, sizeof local_address) < 0)
        syserr("bind");
}
