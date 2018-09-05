#ifndef SIKRADIO_MESSAGES_H
#define SIKRADIO_MESSAGES_H

#include <string>
#include <vector>
#include <regex>
#include <set>

class Reply {
    std::string mcast_addr;
    int data_port;
    std::string name;
public:
    Reply(const std::string &mcast_addr, int data_port, const std::string &name) :
            mcast_addr(mcast_addr), data_port(data_port), name(name) {}
    Reply(std::string message);
    static bool isReply(std::string message);
    std::string toString();

    const std::string &getMcast_addr() const {
        return mcast_addr;
    }

    int getData_port() const {
        return data_port;
    }

    const std::string &getName() const {
        return name;
    }
};

class Rexmit {
    std::vector<uint64_t> packages;
public:
    Rexmit(const std::vector<uint64_t> &packages) : packages(packages) {}
    Rexmit(const std::set<uint64_t> &packagesSet);
    Rexmit(std::string message);

    std::string toString();
    static bool isRexmit(std::string message);

    const std::vector<uint64_t> &getPackages() const {
        return packages;
    }
};

static const int MAX_DATAGRAM_SIZE = 65507;

struct Package {
    uint64_t session_id;
    uint64_t first_byte_num;
    uint8_t audio_data[MAX_DATAGRAM_SIZE];
}__attribute__((__packed__));

const std::string LOOKUP = "ZERO_SEVEN_COME_IN\n";

#endif //SIKRADIO_MESSAGES_H
