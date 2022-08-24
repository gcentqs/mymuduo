#include "channel.h"
#include "pollpoller.h"
#include "logger.h"

#include <assert.h>
#include <algorithm>

using namespace muduo;

PollPoller::PollPoller(EventLoop* loop)
    : Poller(loop) {}

PollPoller::~PollPoller() = default;

TimeStamp PollPoller::poll(int timeout_ms, ChannelList* active_channels_) {
    // LOG_INFO("PollPoller::poll()");
    int num_events = ::poll(pollfds_.data(), pollfds_.size(), timeout_ms);
    int saved_errno = errno;
    TimeStamp now(TimeStamp::now());
    if (num_events > 0) {
        // LOG_INFO("%d events happened\n", num_events);
        fillActiveChannels(num_events, active_channels_);
    } else if (num_events == 0){
        // LOG_INFO("nothing happened");
    } else {
        if (errno != EINTR) {
            LOG_ERROR("PollPoller::poll()");
        }
        errno = saved_errno;
    }
    return now;
}

void PollPoller::fillActiveChannels(int num_events, ChannelList* active_channels) const
{
    for (auto iter_pfd = pollfds_.begin(); iter_pfd != pollfds_.end() && num_events > 0; ++iter_pfd) {
        if (iter_pfd->revents > 0) {
            --num_events;
            ChannelMap::const_iterator iter_ch = channels_.find(iter_pfd->fd);
            assert(iter_ch != channels_.end());
            Channel* channel = iter_ch->second;
            assert(channel->fd() == iter_pfd->fd);
            channel->set_revents(iter_pfd->revents);
            active_channels->push_back(channel);
        } 
    }
}

void PollPoller::updateChannel(Channel* channel) {
    Poller::assertInLoopThread();
    // LOG_INFO("fd = %d update events = %d", channel->fd(), channel->events());
    if (channel->index() < 0) { 
        // 未初始化，新增加一个在pollfds_中
        assert(channels_.count(channel->fd()) ==  0);
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.emplace_back(pfd);
        int idx = static_cast<int>(pollfds_.size()) - 1; 
        channel->set_index(idx);
        channels_[channel->fd()] = channel;
    } else {
        // 更新已存在的channel 
        assert(channels_.count(channel->fd()) != 0);
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        struct pollfd& pfd = pollfds_[idx];
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->isNoneEvent()) {
            pfd.fd = -channel->fd() - 1;    // 没有事件，所以暂时忽略
        }
    }
}

void PollPoller::removeChannel(Channel* channel) {
    Poller::assertInLoopThread();
    // LOG_INFO("PollPoller::removeChannel: fd = %d removed from poller", channel->fd());
    assert(channels_.count(channel->fd()) != 0);
    assert(channels_[channel->fd()] == channel);
    assert(channel->isNoneEvent());

    channels_.erase(channel->fd());

    int idx = channel->index();
    // const struct pollfd& pfd = pollfds_[idx];
    if (idx == pollfds_.size() - 1) {
        pollfds_.pop_back();
    } else {
        int fd_at_end = pollfds_.back().fd;
        std::iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
        if (fd_at_end < 0) {
            fd_at_end = -fd_at_end - 1;
        }
        channels_[fd_at_end]->set_index(idx);
        pollfds_.pop_back();
    }
}
