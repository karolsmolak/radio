#include "DataController.h"

DataController::DataController(int dataPort, const std::string &mcastAddr, int packageSize) : dataPort(dataPort), mcastAddr(mcastAddr), packageSize(packageSize) {
    struct sockaddr_in remote_address;
    dataSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (dataSocket < 0)
        syserr("socket");

    int optval = 1;
    if (setsockopt(dataSocket, SOL_SOCKET, SO_BROADCAST, (void*)&optval, sizeof optval) < 0)
        syserr("setsockopt broadcast");

    optval = 4;
    if (setsockopt(dataSocket, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&optval, sizeof optval) < 0)
        syserr("setsockopt multicast ttl");

    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(dataPort);
    if (inet_aton(mcastAddr.c_str(), &remote_address.sin_addr) == 0)
        syserr("inet_aton");
    if (connect(dataSocket, (struct sockaddr *)&remote_address, sizeof remote_address) < 0)
        syserr("connect");
    emptyMutex.lock();
}

void DataController::sendPackages() {
    while (!finished) {
        while (true) {
            emptyMutex.lock();
            toSendMutex.lock();
            if (toSend.empty()) {
                toSendMutex.unlock();
                break;
            }
            emptyMutex.unlock();
            Package package = toSend.front();
            toSend.pop_front();
            if (toSend.empty()) {
                emptyMutex.lock();
            }
            toSendMutex.unlock();
            if (write(dataSocket, &package, packageSize) < 0) {
                syserr("error writing");
            }
        }
    }
}

void DataController::enqueue(Package package) {
    std::lock_guard<std::mutex> lock(toSendMutex);
    toSend.push_back(package);
    if (toSend.size() == 1) {
        emptyMutex.unlock();
    }
}
