#ifndef SIKRADIO_RETRANSMISSIONCONTROLLER_H
#define SIKRADIO_RETRANSMISSIONCONTROLLER_H


#include "../../messages/messages.h"
#include "CtrlController.h"
#include <set>
#include <mutex>

class CtrlController;
class StationController;

class RetransmissionController {
    int rtime;
    u_int64_t maxReceived = 0;
    uint32_t psize;
    int maxPackagesInBuffer;
    CtrlController *ctrlController;
    StationController *stationController;
    std::mutex lackingPackagesMutex;
    std::set<int> lackingPackages;
    bool hasBufforSpace(uint64_t package);
public:
    RetransmissionController(int rtime) : rtime(rtime) {}
    void newPackage(uint64_t package);
    void sendRequests();
    void restart(uint64_t maxReceived, uint32_t psize);

    void setCtrlController(CtrlController *ctrlController) {
       this->ctrlController = ctrlController;
    }

    void setStationController(StationController *stationController) {
        this->stationController = stationController;
    }
};


#endif //SIKRADIO_RETRANSMISSIONCONTROLLER_H
