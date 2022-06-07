#include "thread.h"
#include "current_thread.h"

#include <assert.h>
#include <semaphore.h>

using namespace muduo::utils;

std::atomic_int Thread::num_created_(0);

Thread::Thread(ThreadFunc func, const std::string& name)
    : started_(false)
    , joined_(false)
    , tid_(0)
    , func_(std::move(func))
    , name_(name)
{
    setDefaultName();
}

Thread::~Thread() {
    if (started_ && !joined_) {
        thread_->detach();
    }
}

void Thread::setDefaultName() {
    int num = ++num_created_;
    if (name_.empty()) {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = std::string(buf);
    }
}

void Thread::start() {
    assert(!started_);
    started_ = true;
    // 需要同步机制，先创建线程，再确定线程id
    sem_t sem;
    ::sem_init(&sem, false, 0);
    thread_ = std::shared_ptr<std::thread>(
        new std::thread( [&]() {
            tid_ = current_thread::tid();
            sem_post(&sem);
            func_();} 
            ));

    // 等待，直到拿到线程id
    sem_wait(&sem);
}

void Thread::join() {
    joined_ = true;
    thread_->join();
}