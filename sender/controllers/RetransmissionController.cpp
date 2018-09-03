#include <iostream>
#include "RetransmissionController.h"

void RetransmissionController::collectRetransmissions() {
    auto duration = std::chrono::milliseconds(rtime);
    while (!finished) {
        std::this_thread::sleep_for(duration);
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        queueLock.lock();
        auto toRetransmit = rexmitQueue;
        rexmitQueue.clear();
        queueLock.unlock();
        std::set<uint64_t> packageSet;
        for (auto &request : toRetransmit) {
            for (uint64_t package : request.getPackages()) {
                if (package % psize == 0) {
                    packageSet.insert(package);
                }
            }
        }
        Package toSend = Package(sessionId, 0);
        for (uint64_t package : packageSet) {
            toSend.first_byte_num = package;
            if (fifo->getBytes(package, psize, toSend.audio_data)) {
                dataController->sendPackage(toSend);
            }
        }
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        duration = std::chrono::milliseconds(rtime) - std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    }
}

void RetransmissionController::retransmit(Rexmit &rexmit) {
    std::lock_guard<std::mutex> lock(queueLock);
    rexmitQueue.push_back(rexmit);
}
