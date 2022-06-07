#ifndef MUTUO_EVENT_LOOP_THREAD_H
#define MUTUO_EVENT_LOOP_THREAD_H

#include "utils/noncopyable.h"
#include "utils/thread.h"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

using muduo::utils::Thread;

namespace muduo
{

class EventLoop;

class EventLoopThread : noncopyable
{
public:
    typedef std::function<void(EventLoop *)> ThreadInitCallback;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                    const std::string& name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void threadFunc();  // 线程函数

private:
    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback thread_init_callback_;
};

}

#endif