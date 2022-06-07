#ifndef MUDUO_UTILS_THREAD_H
#define MUDUO_UTILS_THREAD_H

#include "noncopyable.h"

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>


namespace muduo
{

namespace utils
{

class Thread
{
public:
    typedef std::function<void()> ThreadFunc;
    
    explicit Thread(ThreadFunc, const std::string& name = std::string());
    ~Thread();

    void start();
    void join();

    bool started() { return started_; }
    pid_t tid() { return tid_; }
    const std::string& name() { return name_; }

    static int numCreated() { return num_created_; }

private:
    void setDefaultName();

private:
    std::string name_;
    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    pid_t tid_; // 在线程创建的时候绑定
    ThreadFunc func_;   // 线程回调函数
    static std::atomic_int num_created_;
};

}

}

#endif