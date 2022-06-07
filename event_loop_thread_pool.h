#ifndef MUDUO_EVENT_LOOP_THREAD_POOL_H
#define MUDUO_EVENT_LOOP_THREAD_POOL_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace muduo
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool
{
public:
    typedef std::function<void(EventLoop *)> ThreadInitCallback;

    EventLoopThreadPool(EventLoop* baseloop, const std::string& name_arg);
    ~EventLoopThreadPool();

    void setThreadNum(int num_threads) { num_threads_ = num_threads; }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    const std::string& name() { return name_; }
    bool started() { return started_; }

    EventLoop* getNextLoop();   // 在多reactor模型中，通过轮询方式讲channel分配给不同的eventthread
    std::vector<EventLoop *> getAllLoop();

private:
    EventLoop *baseloop_;
    std::string name_;
    bool started_;
    int next_;
    int num_threads_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
};

}

#endif
