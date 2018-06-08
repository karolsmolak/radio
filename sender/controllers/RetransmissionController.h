#ifndef SIKRADIO_RETRANSMISSIONCONTROLLER_H
#define SIKRADIO_RETRANSMISSIONCONTROLLER_H


#include <thread>
#include <mutex>
#include "../Fifo.h"
#include "../../messages/messages.h"
#include "CtrlController.h"
#include "DataController.h"
#include <boost/log/trivial.hpp>

class Fifo;
class DataController;

class RetransmissionController {
    bool finished = false;
    uint64_t sessionId;
    int psize;
    int rtime;
    Fifo *fifo;
    DataController *dataController;
    std::vector<Rexmit> rexmitQueue;
    std::mutex queueLock;
public:
    RetransmissionController(uint64_t sessionId, int psize, int rtime)
            : sessionId(sessionId), psize(psize), rtime(rtime) {}

    void collectRetransmissions();

    void retransmit(Rexmit &rexmit);

    void setFifo(Fifo *fifo) {
        RetransmissionController::fifo = fifo;
    }

    void setDataConnection(DataController *dataController) {
        RetransmissionController::dataController = dataController;
    }

    void setFinished(bool finished) {
        RetransmissionController::finished = finished;
    }
};


#endif //SIKRADIO_RETRANSMISSIONCONTROLLER_H
