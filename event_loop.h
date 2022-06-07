#ifndef MUDUO_EVENT_LOOP_H
#define MUDUO_EVENT_LOOP_H

#include "utils/timestamp.h"
#include "utils/current_thread.h"
// #include "poller.h"

#include <any>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>


namespace muduo
{

using muduo::utils::TimeStamp;

class Channel;
class Poller;

class EventLoop
{
public:
    typedef std::function<void()> Functor;

    EventLoop();
    ~EventLoop();

    // 只能在创建的线程中运行
    void loop();
    // 退出loop循环
    void quit();

    // 在loop所属的线程中运行上层回调函数
    void runInLoop(Functor cb);
    // 如果不在目标线程，则加入queue中，通过唤醒loop执行
    void queueInLoop(Functor cb);

    void wakeup();

    TimeStamp pollReturnTime() const { return poll_return_time_; } 
    int64_t iteration() { return iteration_; }
    bool eventHandling() { return event_handling_; }

    void setContext(std::any context) { context_ = context; }
    std::any getContext() { return context_; }

    // 调用poller中的对应方法
    void updateChannel(Channel *);
    void removeChannel(Channel *);
    bool hasChannel(Channel *);

    bool isInLoopThread() { return thread_id_ == current_thread::tid(); }
    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }

private:
    void abortNotInLoopThread();
    void handleRead();    // 用于唤醒loop
    void doPendingFunctors();

    void printActiveChannels() const;   // 用于debug

private:
    typedef std::vector<Channel *> ChannelList;

    std::atomic_bool looping_;
    std::atomic_bool quit_;
    std::atomic_bool event_handling_;   // 是否正在处理事件

    int64_t iteration_; // loop()循环次数
    const pid_t thread_id_; // EventLoop所属的线程id
    TimeStamp poll_return_time_;    // poller返回的时间戳
    std::any context_;  // 可供保存的上下文

    std::unique_ptr<Poller> poller_;

    int wakeup_fd_; // 用于mainloop唤醒subloop中的poller, 处理新到来的连接
    std::unique_ptr<Channel> wakeup_channel_;   // wakeup_fd_对应的channel

    ChannelList active_channels_;   //返回poller所关注的所有就绪的channel列表
    Channel* current_active_channel_; 

    std::mutex mutex_;  // 互斥锁，用来保护下面的vector
    std::vector<Functor> pending_functors_;
    std::atomic_bool calling_pending_functors_;
};

}

#endif
