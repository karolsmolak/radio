

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
public:
    DataController(int dataPort, const std::string &mcastAddr, int packageSize);
    void sendPackage(Package &package);
    ~DataController() {
        close(dataSocket);
    }

};


#endif //SIKRADIO_DATACONTROLLER_H
