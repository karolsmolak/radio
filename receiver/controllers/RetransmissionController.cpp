
#include <thread>
#include <boost/log/trivial.hpp>
#include "RetransmissionController.h"

void RetransmissionController::sendRequests() {
    const auto interval = std::chrono::milliseconds(rtime);
    while (true) {
        std::this_thread::sleep_for(interval);
        lackingPackagesMutex.lock();
        if (!lackingPackages.empty()) {
            ctrlController->sendMessage(Rexmit(lackingPackages).toString(), stationController->getCurrentSender().getCtrlAddress());
        }
        lackingPackagesMutex.unlock();
    }
}

bool RetransmissionController::hasBufforSpace(uint64_t package) {
    return maxReceived - package <= (uint64_t)bsize;
}

void RetransmissionController::newPackage(uint64_t package) {
    lackingPackagesMutex.lock();
    if (lackingPackages.find(package) != lackingPackages.end()) {
        lackingPackages.erase(package);
    }
    if (package >= maxReceived + psize) {
        for (uint64_t first_byte = maxReceived + psize ; first_byte < package ; first_byte += psize) {
            lackingPackages.insert(first_byte);
        }
        maxReceived = package;
    }
    for (auto it = lackingPackages.begin() ; it != lackingPackages.end() ; ++it) {
        if (!hasBufforSpace(*it)) {
            lackingPackages.erase(it);
        } else {
            break;
        }
    }
    lackingPackagesMutex.unlock();
}

void RetransmissionController::restart(uint64_t maxReceived, uint32_t psize) {
    lackingPackagesMutex.lock();
    lackingPackages.clear();
    this->maxReceived = maxReceived;
    this->psize = psize;
    lackingPackagesMutex.unlock();
}