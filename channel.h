#ifndef MUDUO_CHANNEL_H
#define MUDUO_CHANNEL_H

#include "utils/timestamp.h"
#include <functional>
#include <memory>

using namespace muduo::utils;

namespace muduo
{

class EventLoop;

class Channel
{
public:
    typedef std::function<void()> EventCallback;
    typedef std::function<void(TimeStamp)> ReadEventCallback;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    void handleEvent(TimeStamp receive_time); // 根据fd中的事件，调用不同的回调函数 

    void setReadCallback(ReadEventCallback cb) { read_event_callback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { write_event_callback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { error_event_callback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { close_event_callback_ = std::move(cb); }

    void tie(const std::shared_ptr<void>&);

    // 设置fd对应的事件状态
    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }

    // 查看fd当前关注的事件
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isReading() const { return events_ & kReadEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }

    int index() const { return index_; }
    void set_index(int index) { index_ = index; }

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt;}

    std::string eventsToString() const;
    std::string reventsToString() const;

    // 从eventloop中移除channel
    void remove();

    EventLoop* ownerLoop() { return loop_; }

private:
    void handleEventWithGuard(TimeStamp receive_time);
    void update();  // 在eventloop中更新fd对应的channel
    static std::string eventsToString(int fd, int evts);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

private:
    EventLoop* loop_;
    const int fd_;

    int events_; // 用户关心的事件
    int revents_;    // 内核设置的就绪事件
    int index_;

    bool event_handling_;   // 是否正在处理事件
    bool added_to_loop_;    // 是否已加入loop
    bool tied_;
    std::weak_ptr<void> tie_;

    ReadEventCallback read_event_callback_;
    EventCallback write_event_callback_;
    EventCallback error_event_callback_;
    EventCallback close_event_callback_;
};

}

#endif