#ifndef MUDUO_UTILS_CALLBACK_H
#define MUDUO_UTILS_CALLBACK_H

#include "timestamp.h"

#include <functional>
#include <memory>

namespace muduo
{

class Buffer;
class TcpConnection;

namespace utils
{


typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void()> TimeCallback;
typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void(const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;
typedef std::function<void(const TcpConnectionPtr&,
                           Buffer*,
                           TimeStamp)> MessageCallback;

void defaultConnectionCallback(const TcpConnectionPtr&);
void defaultMessageCallback(const TcpConnectionPtr& conn,
                            Buffer* buffer,
                            TimeStamp receive_time);

}

}

#endif
