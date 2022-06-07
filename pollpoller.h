#ifndef MUDUO_POLLPOLLER_H
#define MUDUO_POLLPOLLER_H

#include "poller.h"

#include "poll.h"

namespace muduo
{

class PollPoller : public Poller
{
public:
    PollPoller(EventLoop *);
    ~PollPoller() override;

    TimeStamp poll(int timeout_ms, ChannelList* active_channels) override;

    void updateChannel(Channel *) override;
    void removeChannel(Channel *) override;

private:
    void fillActiveChannels(int num_events, ChannelList* active_channels) const;

private:
    typedef std::vector<struct pollfd> PollFdList;
    PollFdList pollfds_;
};

}

#endif