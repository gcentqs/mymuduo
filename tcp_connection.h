#ifndef MUDUO_TCP_CONNECTION_H
#define MUDUO_TCP_CONNECTION_H

#include "utils/buffer.h"
#include "utils/callbacks.h"
#include "utils/inet_address.h"
#include "utils/noncopyable.h"

#include <any>
#include <memory>
#include <string>

// using muduo::utils::InetAddress;
// using muduo::utils::ConnectionCallback;
using namespace muduo::utils;
using namespace muduo;
struct tcp_info;
/* using muduo::utils::Buffer; */

namespace muduo
{

class Channel;
class EventLoop;
class Socket;

class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop,
                  const std::string& name,
                  int sockfd,
                  const InetAddress& local_addr,
                  const InetAddress& peer_addr);
    ~TcpConnection();

    EventLoop* getEventLoop() { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return local_addr_; }
    const InetAddress& peerAddress() const { return peer_addr_; }

    bool connected() const { return state_ == kConnected; }
    bool disconnected() const { return state_ == kDisconnected; }

    void send(const std::string& buf);

    void shutdown();

    void setConnectionCallback(const ConnectionCallback& cb) 
    { connection_callback_ = cb; }
    void setMessageCallback(const MessageCallback& cb)
    { message_callback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { write_complete_callback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb)
    { high_water_mark_callback_ = cb; }
    // internal use only
    void setCloseCallback(const CloseCallback& cb)
    { close_callback_ = cb; }

    // call only once
    void connectEstablished();
    void connectDestroyed();

    Buffer* inputBuffer();
    Buffer* outputBuffer();

private:
    enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting};
    void handleRead(TimeStamp receive_time);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* data, size_t len);
    void shutdownInLoop();

    void setState(StateE s) { state_ = s; }
    std::string stateToString() const;
    void startReadInLoop();
    void stopReadInLoop();

private:

    EventLoop* loop_;
    const std::string name_;
    StateE state_;
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const InetAddress local_addr_;
    const InetAddress peer_addr_; 

    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    HighWaterMarkCallback high_water_mark_callback_;
    CloseCallback close_callback_;
    size_t high_water_mark_;

    std::any context_;
    utils::Buffer input_buffer_;
    utils::Buffer output_buffer_;
};

}

#endif
