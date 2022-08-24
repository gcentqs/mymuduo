#include "channel.h"
#include "epollpoller.h"
#include "utils/logger.h"

#include <assert.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>

using namespace muduo;

static_assert(EPOLLIN == POLLIN, "epoll uses the same flag as poll");
static_assert(EPOLLPRI == POLLPRI, "epoll uses the same flag as poll");
static_assert(EPOLLOUT == POLLOUT, "epoll uses the same flag as poll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll uses the same flag as poll");
static_assert(EPOLLHUP == POLLHUP, "epoll uses the same flag as poll");

namespace
{
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}

EpollPoller::EpollPoller(EventLoop *loop)
    : Poller(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      epoll_events_(kInitEventListSize)
{
    if (epollfd_ < 0) {
        // LOG_INFO("EpollPoller::EpollPoller()");
    }
}

EpollPoller::~EpollPoller() {
    ::close(epollfd_);
}

TimeStamp EpollPoller::poll(int timeout_ms, ChannelList* active_channels) {
    // LOG_INFO("EpollPoller::poll()");
    int num_events = ::epoll_wait(epollfd_,
                                  epoll_events_.data(),
                                  static_cast<int>(epoll_events_.size()),
                                  timeout_ms);
    int saved_errno = errno;
    TimeStamp now(TimeStamp::now());
    if (num_events > 0) {
        // LOG_INFO("%d events happend", num_events);
        fillActiveChanels(num_events, active_channels);
        if (static_cast<size_t>(num_events) == epoll_events_.size()) {
            epoll_events_.resize(epoll_events_.size() * 2);
        }
    } else if (num_events == 0) {
        // LOG_INFO("nothing happened");
    } else {
        if (saved_errno != EINTR) {
            errno = saved_errno;
            LOG_ERROR("EpollPoller::poll() error");
        } 
    }
    return now;
}

void EpollPoller::fillActiveChanels(int num_events, ChannelList* active_channels) const {
    for (int i = 0; i < num_events; ++i) {
        Channel* channel = static_cast<Channel *>(epoll_events_[i].data.ptr);
#ifdef MUDEBUG
        int fd = channel->fd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);
#endif
        channel->set_revents(epoll_events_[i].events);
        active_channels->push_back(channel);
    }
}

void EpollPoller::updateChannel(Channel* channel) {
    Poller::assertInLoopThread();
    // LOG_INFO("fd %d update events = %d", channel->fd(), channel->events());
    int idx = channel->index(); // channel 目前的状态
    if (idx == kNew || idx == kDeleted) {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (idx == kNew) {
            assert(channels_.count(fd) == 0);
            channels_[fd] = channel;
        } else {
            assert(channels_.count(fd));
            assert(channels_[fd] == channel);
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        // udpate existed one with EPOLL_CTL_MOD/DEL
        int fd = channel->fd();
        assert(channels_.count(fd) && channels_[fd] == channel);
        assert(idx == kAdded);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(Channel* channel) {
    Poller::assertInLoopThread();
    int fd = channel->fd();
    // LOG_INFO("EpollPoller::removeChannel(): fd %d removed from epoller", fd);
    assert(channels_.count(fd) && channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int idx = channel->index();
    assert(idx == kAdded || idx == kDeleted);
    channels_.erase(fd);
    if (idx == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kDeleted);
}

void EpollPoller::update(int operation, Channel* channel) {
    struct epoll_event event;
    memset(&event, 0, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_ERROR("epoll_ctl op = %s of %d", operationToString(operation), fd);
        } else {
            LOG_FATAL("epoll_ctl op = %s of %d", operationToString(operation), fd);
        }
    }
}

const char* EpollPoller::operationToString(int op) {
    switch (op) {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_MOD:
            return "MOD";
        case EPOLL_CTL_DEL:
            return "DEL";
        default:
            return "Unknown operation";
    }
}
