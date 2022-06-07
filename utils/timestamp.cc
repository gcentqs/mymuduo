#include <sys/time.h>
#include "timestamp.h"

using namespace muduo::utils;


TimeStamp TimeStamp::now() {
    timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    return TimeStamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

std::string TimeStamp::toString(bool show_microseconds) const {
    time_t seconds = static_cast<time_t>(microseconds_since_epoch_ / kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(microseconds_since_epoch_ % kMicroSecondsPerSecond);

    struct tm tm_time;
    localtime_r(&seconds, &tm_time);

    char buf[64] = {0};
    if (show_microseconds) {
        snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d.%06d",
                tm_time.tm_year + 1900,
                tm_time.tm_mon + 1,
                tm_time.tm_mday,
                tm_time.tm_hour,
                tm_time.tm_min,
                tm_time.tm_sec,
                microseconds);
    } else {
        snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d",
                tm_time.tm_year + 1900,
                tm_time.tm_mon + 1,
                tm_time.tm_mday,
                tm_time.tm_hour,
                tm_time.tm_min,
                tm_time.tm_sec);
    }
    return buf;
}

// #include <iostream>

// int main() {
//     std::cout << TimeStamp::now().toString() << std::endl;
// }
