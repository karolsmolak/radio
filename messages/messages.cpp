#include "messages.h"

std::regex replyRegex(R"(BOREWICZ_HERE\s([\x21-\x7f]+)\s([\x21-\x7f]+)\s([\x20-\x7f]+))");
std::regex rexmitRegex(R"(LOUDER_PLEASE\s(\d+)(,\d+)*)");

Reply::Reply(std::string message) {
    //message must be rexmit
    std::smatch match;
    std::regex_match(message, match, replyRegex);
    mcast_addr = match.str(1);
    data_port = std::stoi(match.str(2));
    name = match.str(3);
}

bool Reply::isReply(std::string message) {
    return std::regex_match(message, replyRegex);
}

std::string Reply::toString() {
    return "BOREWICZ_HERE " + mcast_addr + " " + std::to_string(data_port) + " " + name;
}

bool Rexmit::isRexmit(std::string message) {
    return std::regex_match(message, rexmitRegex);
}

Rexmit::Rexmit(const std::set<uint64_t> &packagesSet) {
    for (uint64_t package : packagesSet) {
        packages.push_back(package);
    }
}

Rexmit::Rexmit(std::string message) {
    std::string dummyString;
    char dummyChar;
    std::stringstream str(message);
    uint64_t package;
    str >> dummyString;
    str >> package;
    packages.push_back(package);
    while (str >> dummyChar >> package) {
        packages.push_back(package);
    }
}

std::string Rexmit::toString() {
    std::string result = "LOUDER_PLEASE ";
    for (size_t i = 0 ; i < packages.size() ; i++) {
        if (i != 0) {
            result += ",";
        }
        result += std::to_string(packages[i]);
    }
    return result;
}
