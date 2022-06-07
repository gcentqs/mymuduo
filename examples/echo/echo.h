#ifndef MUDUO_EXAMPLES_ECHO_H
#define MUDUO_EXAMPLES_ECHO_H

#include <mymuduo/tcp_server.h>

class EchoServer
{
public:
    EchoServer(muduo::EventLoop* loop, const muduo::utils::InetAddress& local_addr);
    ~EchoServer() = default;

    void start();

private:
    void onConnection(const muduo::utils::TcpConnectionPtr& conn);
    void onMessage(const muduo::utils::TcpConnectionPtr& conn,
                   muduo::utils::Buffer* buf,
                   muduo::utils::TimeStamp time);

private:
    muduo::TcpServer server_;
};

#endif