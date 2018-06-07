#include "RetransmissionController.h"

void RetransmissionController::collectRetransmissions() {
    while (!finished) {
        std::this_thread::sleep_for(std::chrono::milliseconds(rtime));
        queueLock.lock();
        if (rexmitQueue.size() > 0) {
            BOOST_LOG_TRIVIAL(info) << "Sending responses for " << rexmitQueue.size() <<" rexmits";
        }
        auto toRetransmit = rexmitQueue;
        rexmitQueue.clear();
        queueLock.unlock();
        std::set<uint64_t> packageSet;
        for (auto request : toRetransmit) {
            for (uint64_t package : request.getPackages()) {
                packageSet.insert(package);
            }
        }
        Package toSend = Package(sessionId, 0);
        for (uint64_t package : packageSet) {
            toSend.first_byte_num = package;
            if (fifo->getBytes(package, psize, toSend.audio_data)) {
                dataController->enqueue(toSend);
            }
        }
    }
}

void RetransmissionController::retransmit(Rexmit &rexmit) {
    queueLock.lock();
    rexmitQueue.push_back(rexmit);
    queueLock.unlock();
}
