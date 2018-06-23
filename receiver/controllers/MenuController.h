#ifndef SIKRADIO_MENUCONTROLLER_H
#define SIKRADIO_MENUCONTROLLER_H

#include <set>
#include <mutex>
#include <map>
#include "../Sender.h"

class StationController;
class DataController;

enum State {
    FIRST,
    SECOND,
    DOWN,
    UP,
    UNKNOWN
};

class MenuAutomata {
    State state = UNKNOWN;
public:
    State nextByte(char byte) {
        if (byte == '\33') {
            state = FIRST;
        } else if (state == FIRST && byte == '\133') {
            state = SECOND;
        } else if (state == SECOND && byte == '\101') {
            state = UP;
        } else if (state == SECOND && byte == '\102') {
            state = DOWN;
        } else {
            state = UNKNOWN;
        }
        return state;
    }
};

class MenuController {
    StationController *stationController;
    unsigned int ui_port;
    std::string getMenuRepresentation();
    void handleConnection(int socket);
    std::set<int> connectionSockets;
    std::mutex connectionSocketsMutex;
    void executeUserAction(State action);
public:
    void processClientConnections();
    void notifyStateChange();

    MenuController(int ui_port) : ui_port(ui_port) {}

    void setStationController(StationController *stationController) {
        MenuController::stationController = stationController;
    }
};

#endif //SIKRADIO_MENUCONTROLLER_H
