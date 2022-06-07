#ifndef MUDUO_SOCKET_H
#define MUDUO_SOCKET_H

#include "utils/noncopyable.h"
#include "utils/inet_address.h"

#include "utils/logger.h"

using muduo::utils::InetAddress;

namespace muduo
{

class Socket : noncopyable
{
public:
    Socket(int sockfd)
        : sockfd_(sockfd) {}
    ~Socket();

    int fd() { return sockfd_; }
    void bindAddress(const InetAddress& local_addr);
    void listen();
    int accept(InetAddress* peer_addr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);

private:
    const int sockfd_;
};

}

#endif