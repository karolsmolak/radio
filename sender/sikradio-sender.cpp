#include <iostream>
#include <boost/program_options.hpp>
#include "../utils/err.h"
#include "controllers/CtrlController.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <future>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

namespace po = boost::program_options;

std::string NAME;
int PSIZE;
int FSIZE;
int CTRL_PORT;
int DATA_PORT;
std::string MCAST_ADDR;
int RTIME;

uint64_t secondsFromEpoch();
void parseArgs(int argc, char *argv[]);

int main(int argc, char* argv[]) {
    parseArgs(argc, argv);
    uint64_t sessionId = secondsFromEpoch();
    DataController dataController(DATA_PORT, MCAST_ADDR, PSIZE + 16);
    Fifo fifo(FSIZE);
    RetransmissionController retransmissionController(sessionId, PSIZE, RTIME);
    CtrlController ctrlController(CTRL_PORT, DATA_PORT, NAME, MCAST_ADDR);

    retransmissionController.setFifo(&fifo);
    retransmissionController.setDataConnection(&dataController);
    ctrlController.setRetransmissionController(&retransmissionController);

    std::thread ctrlWorker = std::thread(&CtrlController::readMessages, &ctrlController);
    std::thread retransmissionWorker = std::thread(&RetransmissionController::collectRetransmissions, &retransmissionController);

    uint64_t byteNum = 0;
    Package package(sessionId, byteNum);
    int byte;
    while ((byte = getchar()) != EOF) {
        fifo.newByte(byte);
        package.audio_data[byteNum % PSIZE] = byte;
        byteNum++;
        if (byteNum % PSIZE == 0) {
            package.first_byte_num = byteNum - PSIZE;
            dataController.sendPackage(package);
        }
    }

    retransmissionController.setFinished(true);
    ctrlController.setFinished(true);
    ctrlWorker.join();
    retransmissionWorker.join();
    return 0;
}

void parseArgs(int argc, char *argv[]) {
    try {
        po::options_description desc{"Opcje"};
        desc.add_options()
                ("help,h", "Help screen")
                ("mcast_addr,a", po::value<std::string>(), "Adres rozglaszania ukierunkowanego")
                ("psize,p", po::value<int>()->default_value(512), "Rozmiar w bajtach paczki")
                ("fsize,f", po::value<int>()->default_value(128 * 1024), "Rozmiar w bajtach kolejki FIFO nadajnika")
                ("ctrl_port,C", po::value<int>()->default_value(30000 + 385978 % 10000), "Port UDP uzywany do transmisji pakietow kontrolnych")
                ("data_port,P", po::value<int>()->default_value(20000 + 385978 % 10000), "Port UDP używany do przesyłania danych")
                ("rtime,R", po::value<int>()->default_value(250), "Czas (w milisekundach) pomiędzy wysłaniem kolejnych raportów o brakujących paczkach (dla odbiorników) oraz czas między kolejnymi retransmisjami paczek.")
                ("name,n", po::value<std::string>()->default_value("Nienazwany Nadajnik"), "Nazwa nadajnika");
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        if (vm.count("help")) {
            std::cout << desc << '\n';
            exit(0);
        }
        if (!vm.count("mcast_addr")) {
            std::cout << "MCAST_ADDR is required\n";
            exit(0);
        }
        PSIZE = vm["psize"].as<int>();
        FSIZE = vm["fsize"].as<int>();
        CTRL_PORT = vm["ctrl_port"].as<int>();
        DATA_PORT = vm["data_port"].as<int>();
        NAME = vm["name"].as<std::string>();
        RTIME = vm["rtime"].as<int>();
        MCAST_ADDR = vm["mcast_addr"].as<std::string>();
    } catch (const po::error &error) {
        std::cerr << error.what() << '\n';
        exit(1);
    }
}

uint64_t secondsFromEpoch() {
    return std::chrono::duration_cast<std::chrono::seconds>
            (std::chrono::system_clock::now().time_since_epoch()).count();
}