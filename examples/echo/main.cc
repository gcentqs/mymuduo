#include "echo.h"
#include <mymuduo/utils/logger.h>
#include <mymuduo/event_loop.h>
#include <mymuduo/utils/inet_address.h>

#include <unistd.h>

int main(int argc, char* argv[])
{
    // LOG_INFO("pid = %d", getpid());
    muduo::EventLoop loop;
    muduo::utils::InetAddress listen_addr(2007);
    EchoServer server(&loop, listen_addr);
    // server.setThreadNum(5);
    server.start();
    loop.loop();
}

