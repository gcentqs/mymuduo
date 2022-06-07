#ifndef MUDUO_ACCEPTOR_H
#define MUDUO_ACCEPTOR_H

#include "utils/noncopyable.h"
#include "channel.h"
#include "socket.h"
#include "utils/timestamp.h"
#include "utils/inet_address.h"

#include <functional>


// using muduo::utils::InetAddress;

namespace muduo
{

using utils::InetAddress;
using utils::TimeStamp;

class EventLoop;

class Acceptor : noncopyable
{
public:
    typedef std::function<void(int sockfd, const InetAddress&)> NewConnectionCallback;

    Acceptor(EventLoop* loop, const InetAddress& listen_addr, bool reuse_port);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb) {
        new_connection_callback_ = cb;
    }

    void listen();
    bool listening() { return listening_; }

private:
    void handleRead();

private:
    EventLoop* loop_;   // baseloop
    Socket accept_socket_;
    Channel accept_channel_;
    bool listening_;
    int idle_fd_;

    NewConnectionCallback new_connection_callback_;
};

}

#endif