#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <future>
#include <boost/program_options.hpp>
#include "../include/DataController.h"
#include "../include/Fifo.h"
#include "../include/RetransmissionController.h"

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
    Package package;
    package.session_id = htobe64(sessionId);
    int byte;
    while ((byte = getchar()) != EOF) {
        fifo.newByte(byte);
        package.audio_data[byteNum % PSIZE] = (uint8_t) byte;
        byteNum++;
        if (byteNum % PSIZE == 0) {
            package.first_byte_num = htobe64(byteNum - PSIZE);
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
                ("mcast_addr,a", po::value<std::string>(&MCAST_ADDR)->required(), "Adres rozglaszania ukierunkowanego")
                ("psize,p", po::value<int>(&PSIZE)->default_value(512), "Rozmiar w bajtach paczki")
                ("fsize,f", po::value<int>(&FSIZE)->default_value(128 * 1024), "Rozmiar w bajtach kolejki FIFO nadajnika")
                ("ctrl_port,C", po::value<int>(&CTRL_PORT)->default_value(30000 + 385978 % 10000), "Port UDP uzywany do transmisji pakietow kontrolnych")
                ("data_port,P", po::value<int>(&DATA_PORT)->default_value(20000 + 385978 % 10000), "Port UDP używany do przesyłania danych")
                ("rtime,R", po::value<int>(&RTIME)->default_value(250), "Czas (w milisekundach) pomiędzy wysłaniem kolejnych raportów o brakujących paczkach (dla odbiorników) oraz czas między kolejnymi retransmisjami paczek.")
                ("name,n", po::value<std::string>(&NAME)->default_value("Nienazwany Nadajnik"), "Nazwa nadajnika");
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        if (vm.count("help")) {
            std::cout << desc << '\n';
            exit(0);
        }
        if (PSIZE <= 0) {
            std::cerr << "Rozmiar paczki musi byc dodatni";
            exit(1);
        }
        if (FSIZE < 0) {
            std::cerr << "Rozmiar kolejki fifo musi byc nieujemny";
            exit(1);
        }
        if (CTRL_PORT <= 0 || DATA_PORT <= 0) {
            std::cerr << "Numer portu musi byc dodatni";
            exit(1);
        }
        if (RTIME < 0) {
            std::cerr << "Czas miedzy retransmisjami musi byc nieujemny";
            exit(1);
        }
    } catch (const po::error &error) {
        std::cerr << error.what() << '\n';
        exit(1);
    }
}

uint64_t secondsFromEpoch() {
    return std::chrono::duration_cast<std::chrono::seconds>
            (std::chrono::system_clock::now().time_since_epoch()).count();
}