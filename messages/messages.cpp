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

bool Rexmit::isRexmit(std::string message) {
    return std::regex_match(message, rexmitRegex);
}