#ifndef MYMUDUO_UTILS_NONCOPYABLE_H
#define MYMUDUO_UTILS_NONCOPYABLE_H


class noncopyable
{
public:
    // 不允许拷贝构造和赋值
    noncopyable(const noncopyable &) = delete;
    noncopyable& operator=(const noncopyable &) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};



#endif