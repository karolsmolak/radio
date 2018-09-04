
#ifndef SIKRADIO_SENDERCTRLCONNECTION_H
#define SIKRADIO_SENDERCTRLCONNECTION_H


#include <string>
#include <netinet/in.h>
#include <cstdlib>
#include <arpa/inet.h>
#include <unistd.h>
#include "../../utils/err.h"
#include "../../messages/messages.h"
#include "RetransmissionController.h"

class RetransmissionController;

class CtrlController {
    int port;
    int ctrlSocket;
    int dataPort;
    std::string name;
    std::string mcastAddr;
    RetransmissionController *retransmissionController;
    bool finished = false;
public:
    void readMessages();

    CtrlController(int ctrlPort, int dataPort, std::string name, std::string mcastAddr);

    void setRetransmissionController(RetransmissionController *retransmissionController) {
        CtrlController::retransmissionController = retransmissionController;
    }

    void setFinished(bool finished) {
        CtrlController::finished = finished;
    }

    ~CtrlController() {
        close(ctrlSocket);
    }
};


#endif //SIKRADIO_SENDERCTRLCONNECTION_H
