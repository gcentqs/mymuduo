#ifndef MYMUDUO_UTILS_TIMESTAMP_H
#define MYMUDUO_UTILS_TIMESTAMP_H

#include <inttypes.h>
#include <string>

namespace muduo
{

namespace utils
{

class TimeStamp {
public:
    explicit TimeStamp(int64_t microseconds = 0) 
        : microseconds_since_epoch_(microseconds) {}

    static TimeStamp now();
    std::string toString(bool = false) const;  // 是否显示到微秒级别

    static const int kMicroSecondsPerSecond = 1000 * 1000; 

private:
    int64_t microseconds_since_epoch_;  // 记录从linux epoch到某一时间的微秒数
};

}

}

#endif