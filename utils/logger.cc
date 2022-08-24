#include "logger.h"
#include "timestamp.h"

#include <string.h>
#include <iostream>
#include <functional>

using namespace muduo::utils;

__thread char t_errorbuf[512];

const char* strerror_tl(int saved_errno) {
    return strerror_r(saved_errno, t_errorbuf, sizeof t_errorbuf);
}


pthread_once_t Logger::ponce_ = PTHREAD_ONCE_INIT;
Logger* Logger::logger_ = nullptr;

Logger& Logger::getInstance() {
    pthread_once(&ponce_, &Logger::init);
    return *logger_;
}

Logger::Logger()
    : log_thread_(std::bind(&Logger::logThreadFunc, this), "LogThread")
{
    log_thread_.start();
}


void Logger::logThreadFunc() {
    while (true) {
        std::string logmsg = popLog();
        // 写入标准输出或者是写入文件
        std::cout << logmsg << std::endl;
    }
}

std::string Logger::popLog() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (log_queue_.empty()) {
        cond_.wait(lock);
    }
    std::string logmsg = log_queue_.front();
    log_queue_.pop();
    return logmsg;
}

void Logger::log(std::string msg) {
    std::string logmsg;
    switch (log_level_) {
    case INFO:
        logmsg = "[INFO]";
        break;
    case ERROR:
        logmsg = "[ERROR]";
        break;
    case FATAL:
        logmsg = "[FATAL]";
        break;
    case DEBUG:
        logmsg = "[DEBUG]";
        break;
    default:
        break;
    }

    // std::cout << TimeStamp::now().toString() << " " << msg << std::endl;
    logmsg += TimeStamp::now().toString() + " " + msg;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        log_queue_.emplace(logmsg);
        cond_.notify_one();
    }
}

