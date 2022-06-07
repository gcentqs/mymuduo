#include "event_loop.h"
#include "event_loop_thread.h"
#include "event_loop_thread_pool.h"

#include <assert.h>


using namespace muduo;


EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop, const std::string& name_arg)
    : baseloop_(baseloop)
    , name_(name_arg)
    , started_(false)
    , next_(0)
    , num_threads_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
    assert(!started_);
    baseloop_->assertInLoopThread();

    started_ = true;

    for (int i = 0; i < num_threads_; ++i) {
        std::string name = name_ + std::to_string(i); 
        // char buf[name_.size() + 32];
        // snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb, name);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop()); 
    }
    if (num_threads_ == 0 && cb) {  // 整个服务端只有一个baseloop作为eventloop
        cb(baseloop_);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    baseloop_->assertInLoopThread();
    assert(started_);

    EventLoop* loop = baseloop_;
    if (!loops_.empty()) {
        loop = loops_[next_++]; 
        if ((size_t)next_ >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoop() {
    baseloop_->assertInLoopThread();
    assert(started_);

    if (loops_.empty()) {
        return {baseloop_};
    } else {
        return loops_;
    }
}

