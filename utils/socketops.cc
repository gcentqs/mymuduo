#include "logger.h"
#include "socketops.h"

#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace muduo;


int sockets::createNonBlockingSocketOrDie(sa_family_t family) {
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0) {
        LOG_ERROR("createNonBlockingSocketOrDie");
    }
    return sockfd;
}

void sockets::close(int sockfd) {
    if (::close(sockfd) < 0) {
        LOG_ERROR("sockets::close failed");
    }
}

ssize_t sockets::write(int sockfd, const void* buf, size_t len) {
    return ::write(sockfd, buf, len);
}

int sockets::getSocketError(int sockfd) {
    int optval;
    socklen_t len = static_cast<socklen_t>(sizeof optval);
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &len) < 0) {
        return errno;
    } else {
        return optval;
    }
}

struct sockaddr_in sockets::getLocalAddress(int sockfd) {
    struct sockaddr_in local;
    ::memset(&local, 0, sizeof local);
    socklen_t len = static_cast<socklen_t>(sizeof local);
    if (::getsockname(sockfd, (sockaddr *)&local, &len) < 0) {
        LOG_ERROR("sockets::getLocalAddress");
    }
    return local;
}
