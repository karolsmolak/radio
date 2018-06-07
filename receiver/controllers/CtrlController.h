#ifndef SIKRADIO_CTRLSERVER_H
#define SIKRADIO_CTRLSERVER_H

#include <string>
#include "StationController.h"

class StationController;

class CtrlController {
    std::string discoverAddr;
    int ctrlPort;
    int ctrlSocket;
    StationController *stationController;
    struct sockaddr_in broadcast_address;
public:
    CtrlController(std::string &discoverAddr, int ctrlPort);

    void sendMessage(std::string message, struct sockaddr_in address);
    void sendMessage(std::string message);
    void readMessages();

    void setStationController(StationController *stationController) {
        CtrlController::stationController = stationController;
    }
};


#endif //SIKRADIO_CTRLSERVER_H
