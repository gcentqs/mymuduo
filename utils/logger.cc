#include "logger.h"
#include "timestamp.h"

#include <string.h>
#include <iostream>

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
    std::cout << TimeStamp::now().toString() << " " << msg << std::endl;
}

