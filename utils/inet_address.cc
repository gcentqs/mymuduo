#include <string.h>
#include <arpa/inet.h>
#include "inet_address.h"

// using muduo::utils::InetAddress;
using namespace muduo::utils;

InetAddress::InetAddress(const uint16_t port, const std::string ip) {
    memset(&addr_, 0, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_port = ::htons(port);
    addr_.sin_addr.s_addr = ::inet_addr(ip.c_str());
}

std::string InetAddress::toIp() const {
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    return buf;
}

std::string InetAddress::toIpPort() const {
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    size_t end = strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf + end, ":%u", port);
    return buf;
}

uint16_t InetAddress::toPort() const {
    uint16_t port = ntohs(addr_.sin_port);
    return port;
}

// #include <iostream>

// int main() {
//     std::string ip{"114.114.114.114"};
//     InetAddress addr(1234, ip);
//     std::cout << addr.toIp() << std::endl
//          << addr.toIpPort() << std::endl
//          << addr.toPort() << std::endl;
// }