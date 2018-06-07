#ifndef SIKRADIO_DATACONTROLLER_H
#define SIKRADIO_DATACONTROLLER_H


#include <thread>
#include "../Sender.h"
#include "CtrlController.h"
#include "../Buffer.h"
#include "RetransmissionController.h"

class RetransmissionController;
class StationController;
class Sender;
class Buffer;

class DataController {
    Buffer *buffer;
    RetransmissionController *retransmissionController;
    StationController *stationController;
    Sender sender;
    bool isPlaying = false;
    std::thread playThread;
public:
    void play();

    void setRetransmissionController(RetransmissionController *retransmissionController) {
        DataController::retransmissionController = retransmissionController;
    }

    void setStationController(StationController *stationController) {
        DataController::stationController = stationController;
    }

    void setBuffer(Buffer *buffer) {
        DataController::buffer = buffer;
    }

    void notifyCurrentSenderChange();
};


#endif //SIKRADIO_DATACONTROLLER_H
