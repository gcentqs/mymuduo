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
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (log_queue_.empty()) {
                cond_.wait(lock);
            }
            std::string logmsg = log_queue_.front();
            log_queue_.pop();
            std::cout << logmsg << std::endl;
        }
    }
}

void Logger::log(std::string msg) {
    switch (log_level_) {
    case INFO:
        std::cout << "[INFO]";
        break;
    case ERROR:
        std::cout << "[ERROR]";
        break;
    case FATAL:
        std::cout << "[FATAL]";
        break;
    case DEBUG:
        std::cout << "[DEBUG]";
        break;
    default:
        break;
    }

    // std::cout << TimeStamp::now().toString() << " " << msg << std::endl;
    std::string logmsg{TimeStamp::now().toString() + " " + msg};
    {
        std::unique_lock<std::mutex> lock(mutex_);
        log_queue_.emplace(logmsg);
        cond_.notify_one();
    }
}

