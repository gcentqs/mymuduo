#include "channel.h"
#include "event_loop.h"
#include "logger.h"

#include <assert.h>
#include <poll.h>

#include <iostream>

using namespace muduo;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      index_(-1),
      event_handling_(false),
      added_to_loop_(false),
      tied_(false) {

}

Channel::~Channel() {
    assert(!event_handling_);
    assert(!added_to_loop_);
    if (loop_->isInLoopThread()) {
        assert(!loop_->hasChannel(this));
    }
}

void Channel::tie(const std::shared_ptr<void>& obj) {
    tie_ = obj;
    tied_ = true;
}

void Channel::update() {
    added_to_loop_ = true;
    loop_->updateChannel(this);
}

void Channel::remove() {
    added_to_loop_ = false;
    loop_->removeChannel(this);
}


void Channel::handleEvent(TimeStamp receive_time) {
    std::shared_ptr<void> guard;
    if (tied_) {
        guard = tie_.lock();
        if (guard) {
            handleEventWithGuard(receive_time);
        }
    } else {
        handleEventWithGuard(receive_time);
    }
}

void Channel::handleEventWithGuard(TimeStamp receive_time) {
    event_handling_ = true;
    LOG_INFO("Channel::handleEvent() handle events %s", reventsToString().c_str());

    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) { // 关闭
        if (close_event_callback_) {
            close_event_callback_();
        }
    }
    if (revents_ & POLLNVAL) {  // fd无效

    }
    if (revents_ & (POLLERR | POLLNVAL)) {
        if (error_event_callback_) {
            error_event_callback_();
        }
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (read_event_callback_) {
            read_event_callback_(receive_time);
        }
    }
    if (revents_ & POLLOUT) {
        if (write_event_callback_) {
            write_event_callback_();
        }
    }
    event_handling_ = false;
}

std::string Channel::reventsToString() const {
    return eventsToString(fd_, revents_); 
}

std::string Channel::eventsToString() const {
    return eventsToString(fd_, events_);
}

std::string Channel::eventsToString(int fd, int evts) {
    std::string s(std::to_string(fd));
    s += ":";
    if (evts & POLLIN) {
        s += "IN ";
    }
    if (evts & POLLPRI) {
        s += "PRI ";
    }
    if (evts & POLLOUT) {
        s += "OUT ";
    }
    if (evts & POLLHUP) {
        s += "HUP ";
    }
    if (evts & POLLRDHUP) {
        s += "RDHUP ";
    }
    if (evts & POLLERR) {
        s += "ERR ";
    }
    if (evts & POLLNVAL) {
        s += "NVAL ";
    }
    return s;
}
