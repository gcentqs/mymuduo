#ifndef MYMUDUO_UTILS_SOCKETOPS_H
#define MYMUDUO_UTILS_SOCKETOPS_H

#include <netinet/in.h>

namespace muduo
{

namespace sockets
{

int createNonBlockingSocketOrDie(sa_family_t family);

// void bindOrDie(int sockfd, const struct sockaddr* addr);   // 失败打印错误日志
// void listenOrDie(int sockfd);
// void acceptOrDie(int sockfd, struct );

// void accept();
// void connect();
void close(int sockfd);   // 关闭socket
// void shutdownWrite();   // 关闭写端

// void read();
// void readv();
ssize_t write(int sockfd, const void* buf, size_t len);

int getSocketError(int sockfd);

struct sockaddr_in getLocalAddress(int sockfd);

}

}

#endif