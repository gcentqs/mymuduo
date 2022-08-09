#include <stdio.h>
#include <string.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include "inet_address.h"
#include "socket.h"

using muduo::Socket;
using muduo::utils::InetAddress;

Socket::~Socket() {
    if (0 != ::close(sockfd_)) {
        LOG_ERROR("%s : %s close failed\n", __FILE__, __FUNCTION__);
    }
}

void Socket::bindAddress(const InetAddress& local_addr) {
    if (0 != ::bind(sockfd_,
        (sockaddr *)(local_addr.getSockAddr()),
        sizeof(sockaddr_in)) ) {
        LOG_FATAL("%s : %s bind address %s failed\n", __FILE__, __FUNCTION__, local_addr.toIpPort().c_str());
    }
}

void Socket::listen() {
    if (0 != ::listen(sockfd_, SOMAXCONN)) {
        LOG_FATAL("%s : %s listen %d failed\n", __FILE__, __FUNCTION__, sockfd_);
    }
}

int Socket::accept(InetAddress* peer_addr) {
    // 使用accept4 
    sockaddr_in addr;
    socklen_t len = sizeof peer_addr;
    // bug!! ::memset(&peer_addr, 0, sizeof peer_addr);
    ::memset(&addr, 0, sizeof addr);
    int connfd = ::accept4(sockfd_, (sockaddr *)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd >= 0) {
        peer_addr->setSockAddr(addr);
    } else {
        LOG_ERROR("%s : %s accept failed\n", __FILE__, __FUNCTION__);
    }
    return connfd;
}

void Socket::shutdownWrite() {
    // 关闭写
    if (::shutdown(sockfd_, SHUT_WR) < 0) {
        LOG_ERROR("%s : %s %d shutdown write failed\n", __FILE__, __FUNCTION__, sockfd_);
    }
}

// 禁用 Nagle算法
// #include<netinet/tcp.h>
void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    if (::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval) < 0) {
        LOG_ERROR("%s : %s set TCP_NODELAY failed\n", __FILE__, __FUNCTION__);
    }
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    if (::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) < 0) {
        LOG_ERROR("%s : %s set SO_REUSEADDR failed\n", __FILE__, __FUNCTION__);
    }
}

void Socket::setReusePort(bool on) {
    int optval = on ? 1 : 0;
    if (::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval) < 0) {
        LOG_ERROR("%s : %s set SO_REUSEPORT failed\n", __FILE__, __FUNCTION__);
    }
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    if (::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval) < 0) {
        LOG_ERROR("%s : %s set SO_KEEPALIVE failed\n", __FILE__, __FUNCTION__);
    }
}