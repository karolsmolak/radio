#ifndef SIKRADIO_STATIONCONTROLLER_H
#define SIKRADIO_STATIONCONTROLLER_H


static const int START_LOOKUP_RETRIES = 6;

#include <mutex>
#include <vector>
#include "Sender.h"
#include "../../messages/messages.h"
#include "CtrlController.h"
#include "DataController.h"

class MenuController;
class CtrlController;
class DataController;

class StationController {
    std::vector<Sender> senders;
    const int MAX_SECONDS_FROM_LAST_RESPONSE = 20;
    CtrlController *ctrlController;
    MenuController *menuController;
    DataController *dataController;
    Sender currentSender;
    bool initialized = false;
    std::string firstSender;
    std::mutex sendersMutex;

    void addSender(Sender sender);
    void sendLookupsOnStart(const std::chrono::duration<int64_t, std::milli> &START_LOOKUP_INTERVAL) const;
public:

    StationController(const std::string &firstSender) : firstSender(firstSender) {}
    void processReply(Reply reply, struct sockaddr_in sender_ctrl_address);
    void sendControllPackets();
    void nextStation();

    void previousStation();

    const std::vector<Sender> &getSenders() {
        return senders;
    }

    bool hasSenders() {
        return !senders.empty();
    }

    Sender getCurrentSender() {
        return currentSender;
    }

    void setCtrlController(CtrlController *ctrlController) {
        this->ctrlController = ctrlController;
    }

    void setMenuController(MenuController *menuController) {
        this->menuController = menuController;
    }

    void setDataController(DataController *dataController) {
        this->dataController = dataController;
    }

    bool isSenderChangeable() const;
};


#endif //SIKRADIO_STATIONCONTROLLER_H
