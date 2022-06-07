#ifndef MUDUO_POLLER_H
#define MUDUO_POLLER_H

#include "utils/noncopyable.h"
#include "utils/timestamp.h"
#include "event_loop.h"

#include <map>
#include <vector>


namespace muduo
{

using utils::TimeStamp;

class Channel;

class Poller :  noncopyable
{
public:
    typedef std::vector<Channel *> ChannelList;
public:
    Poller(EventLoop *loop);
    virtual ~Poller() = default;

    virtual TimeStamp poll(int timeout_ms, ChannelList* active_channels) = 0;

    virtual void updateChannel(Channel *) = 0;
    virtual void removeChannel(Channel *) = 0;
    virtual bool hasChannel(Channel *) const;

    static Poller* newDefaultPoller(EventLoop *);

    // void assertInLoopThread() const;
    void assertInLoopThread() const {
        ownerloop_->assertInLoopThread();
    }

protected:
    typedef std::map<int, Channel*> ChannelMap; // {fd, channel}
    ChannelMap channels_;

private:
    EventLoop* ownerloop_;
};

}

#endif