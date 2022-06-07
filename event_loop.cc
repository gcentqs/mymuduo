#include "channel.h"
#include "current_thread.h"
#include "event_loop.h"
#include "poller.h"
#include "utils/logger.h"

#include <assert.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <mutex>

using namespace muduo;

__thread EventLoop* t_loop_in_this_thread = nullptr;

const int kPollTimeMs = 10000;

int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_FATAL("createEventfd() failed");
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      event_handling_(false),
      iteration_(0),
      thread_id_(current_thread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      wakeup_fd_(createEventfd()),
      wakeup_channel_(new Channel(this, wakeup_fd_)),
      current_active_channel_(nullptr),
      calling_pending_functors_(false)
{
    LOG_DEBUG("EventLoop create %p in thread %d", this, thread_id_);
    if (t_loop_in_this_thread) {
        LOG_FATAL("Another eventloop already exits in this thread %d", current_thread::tid());
    } else {
        t_loop_in_this_thread = this;
    }
    wakeup_channel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeup_channel_->enableReading();
}

EventLoop::~EventLoop() {
    LOG_DEBUG("EventLoop %p of thread %d destruct in thread %d", this, thread_id_, current_thread::tid());
    wakeup_channel_->disableAll();
    wakeup_channel_->remove();
    ::close(wakeup_fd_);
    t_loop_in_this_thread = nullptr;
}

void EventLoop::loop() {
    assertInLoopThread();
    assert(!looping_);
    looping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p starts looping", this);

    while (!quit_) {
        active_channels_.clear();
        poll_return_time_ = poller_->poll(kPollTimeMs, &active_channels_);
        ++iteration_;
        event_handling_ = true;
#ifdef MUDEBUG
        printActiveChannels();
#endif
        for (auto &channel : active_channels_) {
            current_active_channel_ = channel;
            current_active_channel_->handleEvent(poll_return_time_);
        } 
        current_active_channel_ = nullptr;
        event_handling_ = false;
        doPendingFunctors();
    }
    looping_ = false;
    LOG_INFO("EventLoop %p stops looping", this);
}

void EventLoop::quit() {
    assert(!looping_);
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pending_functors_.emplace_back(cb);
    }
    if (!isInLoopThread() || calling_pending_functors_) {
        // 防止这一轮loop之后，poller陷入阻塞，从而导致该cb无法执行
        wakeup();
    }
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = ::write(wakeup_fd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR("Event::wakeup write %d bytes of %d", n, sizeof one);
    }
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = ::read(wakeup_fd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR("Event::handleWakeup read %d bytes of %d", n, sizeof one);
    }
}

void EventLoop::updateChannel(Channel* channel) {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel) {
    return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
    LOG_FATAL("EventLoop::abortNotInLoopThread - \
               EventLoop %p was created in thread \
               %d but called in current thread %d",
               this, thread_id_, current_thread::tid());
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    calling_pending_functors_ = true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pending_functors_);
    }

    for (const Functor& functor : functors) {
        functor();
    }
    calling_pending_functors_ = false;
}

void EventLoop::printActiveChannels() const {
    for (const Channel* channel : active_channels_) {
        LOG_DEBUG("{%s}", channel->reventsToString().c_str());
    }
}