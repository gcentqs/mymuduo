#include "acceptor.h"
#include <event_loop.h>
#include "utils/inet_address.h"
#include "utils/socketops.h"

#include <fcntl.h>
#include <unistd.h>

#include <functional>

using namespace muduo;
using namespace utils;


Acceptor::Acceptor(EventLoop* loop, const InetAddress& listen_addr, bool reuse_port)
    : loop_(loop)
    // 根据本地地址，创建监听的socket
    , accept_socket_(sockets::createNonBlockingSocketOrDie(listen_addr.family()))
    // 根据监听socket，创建对应的channel
    , accept_channel_(loop, accept_socket_.fd())
    , listening_(false)
    , idle_fd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    accept_socket_.setReuseAddr(true); 
    accept_socket_.setReusePort(reuse_port); 
    accept_socket_.bindAddress(listen_addr);
    accept_channel_.setReadCallback(
        std::bind(&Acceptor::handleRead, this)
    );
}

Acceptor::~Acceptor() {
    accept_channel_.disableAll();
    accept_channel_.remove();
    ::close(idle_fd_);
}

void Acceptor::listen() {
    loop_->assertInLoopThread();
    listening_ = true;
    accept_socket_.listen();
    accept_channel_.enableReading();
}

void Acceptor::handleRead() {
    loop_->assertInLoopThread();
    InetAddress peer_addr;
    int connfd = accept_socket_.accept(&peer_addr);
    if (connfd >= 0) {
        if (new_connection_callback_) {
            new_connection_callback_(connfd, peer_addr);
        } else {
            sockets::close(connfd);
        }
    } else {
        LOG_ERROR("Acceptor::handleRead accept failed");
        if (errno == EMFILE) {  // 文件描述符过多
            ::close(idle_fd_);
            idle_fd_ = ::accept(accept_socket_.fd(), NULL, NULL);
            ::close(idle_fd_);
            idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}