

#ifndef SIKRADIO_DATACONTROLLER_H
#define SIKRADIO_DATACONTROLLER_H


#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <mutex>
#include "../../messages/messages.h"
#include "../../utils/err.h"


class DataController {
    int dataSocket;
    int dataPort;
    std::string mcastAddr;
    int packageSize;
    std::deque<Package> toSend;
    bool finished = false;
    std::mutex toSendMutex;
    std::mutex emptyMutex;
public:
    DataController(int dataPort, const std::string &mcastAddr, int packageSize);

    void sendPackages();

    void enqueue(Package package);

    void setFinished(bool finished) {
        DataController::finished = finished;
        emptyMutex.unlock();
    }

    ~DataController() {
        close(dataSocket);
    }
};


#endif //SIKRADIO_DATACONTROLLER_H
