#ifndef MUDUO_TCP_SERVER_H
#define MUDUO_TCP_SERVER_H

#include "acceptor.h"
#include "event_loop_thread_pool.h"
#include "tcp_connection.h"
#include "utils/noncopyable.h"

#include <atomic>
#include <unordered_map>

namespace muduo
{

class EventLoop;

class TcpServer : noncopyable
{
public:
    typedef std::function<void(EventLoop *)> ThreadInitCallback;

    enum Option { kNoReusePort, kReusePort };
    TcpServer(EventLoop* loop,
              const InetAddress& listen_addr,
              const std::string& name_arg,
              Option opt = kNoReusePort);
    ~TcpServer();

    const std::string& ipPort() { return ip_port_; }
    const std::string& name() { return name_; }
    EventLoop* getLoop() { return loop_; }

    void setThreadNum(int num_threads);
    void setThreadInitCallback(ThreadInitCallback& cb)
    { thread_init_callback_ = cb; }

    std::shared_ptr<EventLoopThreadPool> threadPool() 
    { return thread_pool_; }

    void start();

    void setConnectionCallback(const ConnectionCallback& cb)
    { connection_callback_ = cb; }
    void setMessageCallback(const MessageCallback& cb)
    { message_callback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { write_complete_callback_ = cb; }

private:
    void newConnection(int sockfd, const InetAddress& peer_addr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

private:
    typedef std::unordered_map<std::string, TcpConnectionPtr> ConnectionMap;

    EventLoop* loop_;   // the acceptor loop(baseloop)
    const std::string ip_port_;
    const std::string name_;

    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> thread_pool_;  // one loop per thread

    std::atomic_int32_t started_;

    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    ThreadInitCallback thread_init_callback_;

    int next_conn_id_;  // 用于命名
    ConnectionMap connections_;
};

}

#endif
