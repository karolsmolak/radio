#include <unistd.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <arpa/inet.h>
#include <cstring>
#include <thread>
#include <vector>
#include <boost/log/trivial.hpp>
#include <iostream>
#include "../../utils/err.h"
#include "MenuController.h"
#include "StationController.h"
#include "../Sender.h"

std::string MenuController::getMenuRepresentation() {
    std::vector<Sender> available_senders = stationController->getSenders();
    Sender currentSender = stationController->getCurrentSender();
    std::string result = "------------------------------------------------------------------------\n\r"
                         "  SIK Radio\n\r"
                         "------------------------------------------------------------------------\n\r";
    for (auto &available_sender : available_senders) {
        result += "  ";
        if (currentSender == available_sender) {
            result += "> ";
        } else {
            result += "  ";
        }
        result += available_sender.getName() + "\n\r";
    }
    result += "------------------------------------------------------------------------\n\r";
    return result;
}

inline void send_message(std::string message, int socket) {
    if (write(socket, message.c_str(), message.length()) < 0) {
        syserr("error writing");
    }
}

inline void clearTerminal(int socket) {
    //Cursor movement: http://tldp.org/HOWTO/Bash-Prompt-HOWTO/x361.html
    send_message("\33[2J\33[0;0H", socket);
}

inline void setTelnetOptions(int socket) {
    //https://stackoverflow.com/questions/273261/force-telnet-client-into-character-mode
    send_message("\377\373\1\377\373\3\377\374\42", socket);
}

void MenuController::handleConnection(int socket) {
    setTelnetOptions(socket);
    clearTerminal(socket);
    send_message(getMenuRepresentation(), socket);
    unsigned char line[100];
    memset(line, 0, sizeof(line));
    MenuAutomata automata;
    while(true) {
        int ret = (int)read(socket, line, sizeof(line));
        for (int i = 0 ; i < ret ; i++) {
            executeUserAction(automata.nextByte(line[i]));
        }
        //todo: sprawdzic kod bledu
        if (ret == -1) {
            perror("read");
        } else if (ret == 0) {
            break;
        }
    }
    connectionSocketsMutex.lock();
    connectionSockets.erase(socket);
    connectionSocketsMutex.unlock();
    close(socket);
}

void MenuController::processClientConnections() {
    int ear, rc;
    socklen_t len;
    struct sockaddr_in server;

    ear = socket(PF_INET, SOCK_STREAM, 0);
    if (ear == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(ui_port);
    rc = bind(ear, (struct sockaddr *)&server, sizeof(server));
    if (rc == -1) {
        perror("bind");
        exit(1);
    }

    len = (socklen_t)sizeof(server);
    rc = getsockname(ear, (struct sockaddr *)&server, &len);
    if (rc == -1) {
        perror("getsockname");
        exit(EXIT_FAILURE);
    }

    rc = listen(ear, 5);
    if (rc == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (true) {
        int msgsock = accept(ear, (struct sockaddr *)NULL, NULL);
        if (msgsock == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        connectionSocketsMutex.lock();
        connectionSockets.insert(msgsock);
        connectionSocketsMutex.unlock();
        std::thread([this, msgsock]{handleConnection(msgsock);}).detach();
    }
}

void MenuController::notifyStateChange() {
    std::lock_guard<std::mutex> lock(connectionSocketsMutex);
    for (int connectionSocket : connectionSockets) {
        clearTerminal(connectionSocket);
        send_message(getMenuRepresentation(), connectionSocket);
    }
}

void MenuController::executeUserAction(State action) {
    if (action == DOWN) {
        stationController->nextStation();
    } else if (action == UP) {
        stationController->previousStation();
    }
}
