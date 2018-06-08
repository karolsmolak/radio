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
    int bsize;
    uint64_t maxReceived = 0;
    uint32_t psize;
    CtrlController *ctrlController;
    StationController *stationController;
    std::mutex lackingPackagesMutex;
    std::set<uint64_t> lackingPackages;
    bool hasBufforSpace(uint64_t package);
public:
    RetransmissionController(int rtime, int bsize) : rtime(rtime), bsize(bsize) {}
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
