#ifndef MUDUO_UTILS_INET_ADDRESS_H
#define MUDUO_UTILS_INET_ADDRESS_H

#include <inttypes.h>
#include <netinet/in.h>
#include <string>

namespace muduo
{

namespace utils 
{

class InetAddress
{
public:
    InetAddress(const uint16_t port = 0, const std::string ip = "127.0.0.1");
    InetAddress(const struct sockaddr_in& addr) 
        :addr_(addr) {}

    ~InetAddress() {}

    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;

    const sockaddr_in* getSockAddr() const { return &addr_; }
    void setSockAddr(const sockaddr_in& addr) { addr_ = addr; }
    sa_family_t family() const { return addr_.sin_family; }

private:
    sockaddr_in addr_;
};

}

}

#endif