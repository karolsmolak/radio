#include <iostream>
#include <boost/program_options.hpp>
#include "../../utils/err.h"
#include "../include/StationController.h"
#include "../include/DataController.h"
#include "../include/MenuController.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <thread>
#include <list>
#include <chrono>
#include <mutex>
#include <sys/fcntl.h>
#include <future>

namespace po = boost::program_options;

void parseArgs(int argc, char **argv);
std::string DISCOVER_ADDR; //adres używany do wykrywania aktywnych nadajników
int CTRL_PORT; //port UDP używany do transmisji pakietów kontrolnych
int UI_PORT; //port TCP, na którym udostępniany jest prosty interfejs tekstowy do przełączania się między stacjami
int BSIZE; //rozmiar w bajtach bufora
int RTIME; //przerwa pomiędzy kolejnymi prośbami o retransmisje
std::string FIRST_STATION; //pierwsza stacja

int main(int argc, char* argv[]) {
    parseArgs(argc, argv);

    CtrlController ctrlController(DISCOVER_ADDR, CTRL_PORT);
    Buffer buffer(BSIZE);
    RetransmissionController retransmissionController(RTIME, BSIZE);
    StationController stationController(FIRST_STATION);
    DataController dataController;
    MenuController menuController(UI_PORT);

    ctrlController.setStationController(&stationController);

    retransmissionController.setCtrlController(&ctrlController);
    retransmissionController.setStationController(&stationController);

    stationController.setCtrlController(&ctrlController);
    stationController.setMenuController(&menuController);
    stationController.setDataController(&dataController);

    dataController.setRetransmissionController(&retransmissionController);
    dataController.setBuffer(&buffer);
    dataController.setStationController(&stationController);

    menuController.setStationController(&stationController);

    std::thread ctrl(&CtrlController::readMessages, &ctrlController);
    std::thread retransmission(&RetransmissionController::sendRequests, &retransmissionController);
    std::thread station(&StationController::sendControllPackets, &stationController);
    menuController.processClientConnections();
    ctrl.join();
    retransmission.join();
    station.join();
    return 0;
}

void parseArgs(int argc, char **argv) {
    try {
        po::options_description desc{"Opcje"};
        desc.add_options()
        ("help,h", "Help screen")
        ("discover_addr,d", po::value<std::string>(&DISCOVER_ADDR)->default_value("255.255.255.255"), "Adres używany przez odbiornik do wykrywania aktywnych nadajników")
        ("ui_port,U", po::value<int>(&UI_PORT)->default_value(10000 + 385978 % 10000), "Port TCP, na którym udostępniany jest interfejs tekstowy")
        ("ctrl_port,C", po::value<int>(&CTRL_PORT)->default_value(30000 + 385978 % 10000), "Port UDP używany do transmisji pakietów kontrolnych")
        ("bsize,b", po::value<int>(&BSIZE)->default_value(64 * 1024), "Rozmiar w bajtach bufora")
        ("rtime,R", po::value<int>(&RTIME)->default_value(250), "Czas pomiędzy retransmisjami")
        ("first_station,n",po::value<std::string>(&FIRST_STATION)->default_value(""), "Nazwa stacji");
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        if (vm.count("help")) {
            std::cout << desc << '\n';
            exit(0);
        }
        if (CTRL_PORT <= 0) {
            std::cerr << "BLAD: port kontrolny <= 0";
            exit(1);
        }
        if (UI_PORT <= 0) {
            std::cerr << "BLAD: port interfejsu <= 0";
            exit(1);
        }
        if (BSIZE <= 0) {
            std::cerr << "BLAD: rozmiar buforu <= 0";
            exit(1);
        }
        if (RTIME < 0) {
            std::cerr << "BLAD: czas retransmisji < 0";
            exit(1);
        }
    } catch (const po::error &error) {
        std::cerr << error.what() << '\n';
        exit(1);
    }
}