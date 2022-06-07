#ifndef MUDUO_EPOLLPOLLER_H
#define MUDUO_EPOLLPOLLER_H

#include "timestamp.h"
#include "poller.h"

#include <sys/epoll.h>

namespace muduo
{

class EpollPoller : public Poller
{
public:
    EpollPoller(EventLoop *);
    ~EpollPoller() override;

    TimeStamp poll(int timeout_ms, ChannelList* active_channels) override;

    void updateChannel(Channel *) override;
    void removeChannel(Channel *) override;

private:
    void fillActiveChanels(int num_events, ChannelList* active_channels) const;

    static const char* operationToString(int op);
    void update(int operation, Channel* );

private:
    typedef std::vector<struct epoll_event> EpollEventList; 
    static const int kInitEventListSize = 16;

    int epollfd_;
    EpollEventList epoll_events_;
};

}

#endif