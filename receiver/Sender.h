#ifndef SIKRADIO_SENDER_H
#define SIKRADIO_SENDER_H

#include <string>
#include <chrono>
#include "../utils/err.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ostream>
#include <boost/log/trivial.hpp>

class Sender {
    std::string name;
    std::string mcastAddr;
    int data_port;
    int dataSocket;
    struct sockaddr_in ctrlAddress;
    struct ip_mreq ip_mreq;
    std::chrono::system_clock::time_point lastResponse;
public:
    Sender(const std::string &name, const std::string &mcast_addr, int data_port, struct sockaddr_in ctrlAddress) : name(name), mcastAddr(mcast_addr), data_port(data_port), ctrlAddress(ctrlAddress) {
        update();
    }

    void initializeDataSocket() {
        struct sockaddr_in local_address;

        dataSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (dataSocket < 0)
            syserr("socket");

        ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        if (inet_aton(mcastAddr.c_str(), &ip_mreq.imr_multiaddr) == 0)
            syserr("inet_aton");
        if (setsockopt(dataSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&ip_mreq, sizeof ip_mreq) < 0)
            syserr("setsockopt");

        char loopch = 1;

        if(setsockopt(dataSocket, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch, sizeof(loopch)) < 0) {
            perror("Setting IP_MULTICAST_LOOP error");
        }

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if (setsockopt(dataSocket, SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv)) < 0) {
            perror("Error");
        }

        local_address.sin_family = AF_INET;
        local_address.sin_addr.s_addr = htonl(INADDR_ANY);
        local_address.sin_port = htons(data_port);
        if (bind(dataSocket, (struct sockaddr *)&local_address, sizeof local_address) < 0)
            syserr("bind");
    }

    Sender() {}

    const std::string &getName() const {
        return name;
    }

    int getDataSocket() const {
        return dataSocket;
    }

    int getData_port() const {
        return data_port;
    }

    int secondsFromLastResponse() {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastResponse).count();
    }

    void update() {
        lastResponse = std::chrono::system_clock::now();
    }

    ~Sender() {
        close(dataSocket);
    }

    const sockaddr_in &getCtrlAddress() const {
        return ctrlAddress;
    }

    const std::string &getMcastAddr() const {
        return mcastAddr;
    }

    friend std::ostream &operator<<(std::ostream &os, const Sender &sender) {
        os << "name: " << sender.name << " mcast_addr: " << sender.mcastAddr << " data_port: " << sender.data_port;
        return os;
    }

    bool operator<(const Sender &rhs) const {
        for (int i = 0 ; i < std::min(name.length(), rhs.name.length()) ; i++) {
            int a = std::tolower(name[i]);
            int b = std::tolower(rhs.name[i]);
            if (a != b) {
                return a < b;
            }
        }
        return name.length() < rhs.name.length();
    }

    bool operator>(const Sender &rhs) const {
        return rhs < *this;
    }

    bool operator<=(const Sender &rhs) const {
        return !(rhs < *this);
    }

    bool operator>=(const Sender &rhs) const {
        return !(*this < rhs);
    }

    bool operator==(const Sender &rhs) const {
        return mcastAddr == rhs.mcastAddr &&
               data_port == rhs.data_port;
    }

    bool operator!=(const Sender &rhs) const {
        return !(rhs == *this);
    }

    void closeDataSocket() {
        if (setsockopt(dataSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&ip_mreq, sizeof ip_mreq) < 0)
            syserr("setsockopt");
        close(dataSocket);
    }
};

#endif //SIKRADIO_SENDER_H
