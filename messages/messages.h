#ifndef SIKRADIO_MESSAGES_H
#define SIKRADIO_MESSAGES_H

#include <string>
#include <vector>
#include <regex>
#include <set>
#include <boost/log/trivial.hpp>

class Reply {
    std::string mcast_addr;
    int data_port;
    std::string name;
public:
    Reply(const std::string &mcast_addr, int data_port, const std::string &name) : mcast_addr(mcast_addr),
                                                                                   data_port(data_port), name(name) {}

    Reply(std::string message);
    static bool isReply(std::string message);

    std::string toString() {
        return "BOREWICZ_HERE " + mcast_addr + " " + std::to_string(data_port) + " " + name;
    }

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
    std::vector<int> packages;
public:
    Rexmit(const std::vector<int> &packages) : packages(packages) {}

    Rexmit(const std::set<int> &packagesSet) {
        for (int package : packagesSet) {
            packages.push_back(package);
        }
    }

    Rexmit(std::string message) {
        std::string dummyString;
        char dummyChar;
        std::stringstream str(message);
        int package;
        str >> dummyString;
        str >> package;
        packages.push_back(package);
        while (str >> dummyChar >> package) {
            packages.push_back(package);
        }
    }

    std::string toString() {
        std::string result = "LOUDER_PLEASE ";
        for (size_t i = 0 ; i < packages.size() ; i++) {
            if (i != 0) {
                result += ",";
            }
            result += std::to_string(packages[i]);
        }
        return result;
    }

    static bool isRexmit(std::string message);

    const std::vector<int> &getPackages() const {
        return packages;
    }
};

struct Package {
    uint64_t session_id;
    uint64_t first_byte_num;
    unsigned char audio_data[10000];

    Package() {}

    Package(uint64_t session_id, uint64_t first_byte_num) :
            session_id(session_id), first_byte_num(first_byte_num) {}
};

const std::string LOOKUP = "ZERO_SEVEN_COME_IN";

#endif //SIKRADIO_MESSAGES_H
