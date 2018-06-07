#include <iostream>
#include <boost/program_options.hpp>
#include "../utils/err.h"
#include "controllers/StationController.h"
#include "controllers/DataController.h"
#include "controllers/MenuController.h"
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
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

namespace po = boost::program_options;
namespace logging = boost::log;

void parseArgs(int argc, char **argv);
std::string DISCOVER_ADDR; //adres używany do wykrywania aktywnych nadajników
int CTRL_PORT; //port UDP używany do transmisji pakietów kontrolnych
int UI_PORT; //port TCP, na którym udostępniany jest prosty interfejs tekstowy do przełączania się między stacjami
int BSIZE; //rozmiar w bajtach bufora
int RTIME; //przerwa pomiędzy kolejnymi prośbami o retransmisje
std::string FIRST_STATION; //pierwsza stacja


int main(int argc, char* argv[]) {
    srand(time(NULL));
    logging::core::get()->set_filter
            (
                    logging::trivial::severity > logging::trivial::info
            );


    BOOST_LOG_TRIVIAL(info) << "Parsing arguments";
    parseArgs(argc, argv);

    BOOST_LOG_TRIVIAL(info) << "Initializing components";
    CtrlController ctrlController(DISCOVER_ADDR, CTRL_PORT);
    Buffer buffer(BSIZE);
    RetransmissionController retransmissionController(RTIME);
    StationController stationController(FIRST_STATION);
    DataController dataController;
    MenuController menuController(UI_PORT);

    BOOST_LOG_TRIVIAL(info) << "Setting dependencies";
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
    return 0;
}

void parseArgs(int argc, char **argv) {
    try {
        po::options_description desc{"Opcje"};
        desc.add_options()
        ("help,h", "Help screen")
        ("discover_addr,a", po::value<std::string>()->default_value("255.255.255.255"), "Adres używany przez odbiornik do wykrywania aktywnych nadajników")
        ("ui_port,U", po::value<int>()->default_value(10000 + 385978 % 10000), "Port TCP, na którym udostępniany jest interfejs tekstowy")
        ("ctrl_port,C", po::value<int>()->default_value(30000 + 385978 % 10000), "Port UDP używany do transmisji pakietów kontrolnych")
        ("bsize,b", po::value<int>()->default_value(64 * 1024), "Rozmiar w bajtach bufora")
        ("rtime,R", po::value<int>()->default_value(250), "Czas pomiędzy retransmisjami")
        ("first_station,n",po::value<std::string>()->default_value(""), "Nazwa stacji");
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        if (vm.count("help")) {
            std::cout << desc << '\n';
            exit(0);
        }
        CTRL_PORT = vm["ctrl_port"].as<int>();
        UI_PORT = vm["ui_port"].as<int>();
        BSIZE = vm["bsize"].as<int>();
        RTIME = vm["rtime"].as<int>();
        FIRST_STATION = vm["first_station"].as<std::string>();
        DISCOVER_ADDR = vm["discover_addr"].as<std::string>();
    } catch (const po::error &error) {
        std::cerr << error.what() << '\n';
        exit(1);
    }
}