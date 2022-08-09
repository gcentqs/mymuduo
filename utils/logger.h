#ifndef MYMUDUO_UTILS_LOGGER_H
#define MYMUDUO_UTILS_LOGGER_H

#include "noncopyable.h"
#include "thread.h"

#include <pthread.h>
#include <stdio.h>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>


// namespace muduo
// {

// namespace utils
// {
using muduo::utils::Thread;

enum LogLevel
{
    INFO,   // 普通信息
    ERROR,  // 错误信息
    FATAL,  // core dump信息
    DEBUG,  // 调试信息
};

const int kMaxLogBufLength = 1024;

class Logger : noncopyable
{
public:
    static Logger& getInstance();

    void setLogLevel(int level) { log_level_ = level;}

    void log(std::string msg);

private:
    Logger();
    ~Logger() {}

    void logThreadFunc();

    static void init() {
        logger_ = new Logger();
    }

    static pthread_once_t ponce_;
    static Logger* logger_;
    int log_level_;

    // 与 log 队列相关的对象
    Thread log_thread_;
    std::queue<std::string> log_queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

// } // utils

// } // muduo

const char* strerror_tl(int saved_errno);


#define LOG_INFO(logmsg, ...)                                       \
    do {                                                            \
        Logger& logger = Logger::getInstance();               \
        logger.setLogLevel(INFO);                                   \
        char buf[kMaxLogBufLength] = {0};                           \
        snprintf(buf, kMaxLogBufLength, logmsg, ##__VA_ARGS__);     \
        logger.log(buf);                                            \
    } while (0)                                                     

#define LOG_ERROR(logmsg, ...)                                      \
    do {                                                            \
        Logger& logger = Logger::getInstance();                     \
        logger.setLogLevel(ERROR);                                  \
        char buf[kMaxLogBufLength] = {0};                           \
        snprintf(buf, kMaxLogBufLength, logmsg, ##__VA_ARGS__);     \
        logger.log(buf);                                            \
    } while (0)                                                     

#define LOG_FATAL(logmsg, ...)                                      \
    do {                                                            \
        Logger& logger = Logger::getInstance();                     \
        logger.setLogLevel(FATAL);                                  \
        char buf[kMaxLogBufLength] = {0};                           \
        snprintf(buf, kMaxLogBufLength, logmsg, ##__VA_ARGS__);     \
        logger.log(buf);                                            \
        exit(-1);                                                   \
    } while (0)                                                     

#ifdef MUDEBUG
#define LOG_DEBUG(logmsg, ...)                                      \
    do {                                                            \
        Logger& logger = Logger::getInstance();                     \
        logger.setLogLevel(DEBUG);                                  \
        char buf[kMaxLogBufLength] = {0};                           \
        snprintf(buf, kMaxLogBufLength, logmsg, ##__VA_ARGS__);     \
        logger.log(buf);                                            \
    } while (0)                                                     
#else                                                   
#define LOG_DEBUG(logmsg, ...)  // 不做任何处理                     
#endif


// int main() {
//     // Logger::getInstance();
//     LOG_DEBUG();
// }

#endif // MYMUDUO_UTILS_LOGGER_H