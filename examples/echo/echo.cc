#include "echo.h"

#include <functional>

using namespace std::placeholders;

using namespace muduo;
using namespace muduo::utils;


EchoServer::EchoServer(muduo::EventLoop* loop, const muduo::utils::InetAddress& local_addr)
    : server_(loop, local_addr, "EchoServer")
{
    server_.setConnectionCallback(
        std::bind(&EchoServer::onConnection, this, _1)
    );
    server_.setMessageCallback(
        std::bind(&EchoServer::onMessage, this, _1, _2, _3)
    );
}

void EchoServer::start() {
    server_.start();
}

void EchoServer::onConnection(const muduo::utils::TcpConnectionPtr& conn) {
    LOG_INFO("EchoServer : %s -> %s is %s", conn->peerAddress().toIpPort().c_str(),
                                            conn->localAddress().toIpPort().c_str(),
                                            (conn->connected()) ? "UP" : "DOWN");
}

void EchoServer::onMessage(const muduo::utils::TcpConnectionPtr& conn,
                           muduo::utils::Buffer* buf,
                           muduo::utils::TimeStamp time) {
    std::string msg(buf->retriveAllAsString());
    LOG_INFO("%s echo %d bytes data received at time %s",
              conn->name().c_str(),
              msg.size(),
              time.toString().c_str());
    conn->send(msg);
}

