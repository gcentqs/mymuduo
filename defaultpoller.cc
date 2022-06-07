#include "poller.h"
#include "pollpoller.h"
#include "epollpoller.h"

#include "stdlib.h"

using namespace muduo;

Poller* Poller::newDefaultPoller(EventLoop* loop) {
    if (::getenv("MUDUO_USE_POLL")) {
        return new PollPoller(loop);
    }  else {
        return new EpollPoller(loop);
    }
}

