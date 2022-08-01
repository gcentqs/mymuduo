#include "event_loop.h"
#include "tcp_connection.h"
#include "utils/logger.h"
#include "utils/socketops.h"
#include "socket.h"
#include "channel.h"

#include <assert.h>
#include <string.h>
#include <functional>

using namespace muduo;
using namespace std::placeholders;
// using namespace muduo::utils;


void utils::defaultConnectionCallback(const TcpConnectionPtr& conn) {
    LOG_INFO("%s -> %s : %s", conn->localAddress().toIpPort().c_str(),
                              conn->peerAddress().toIpPort().c_str(),
                              (conn->connected() ? "UP" : "Down"));
}

void utils::defaultMessageCallback(const TcpConnectionPtr& conn,
                                   Buffer* buf,
                                   TimeStamp receive_time) {
    // buf->
}

TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& name,
                             int sockfd,
                             const InetAddress& local_addr,
                             const InetAddress& peer_addr)
    : loop_(loop)
    , name_(name)
    , state_(kConnecting)
    , reading_(true)
    , socket_(new Socket(sockfd))
    , channel_(new Channel(loop, sockfd))
    , local_addr_(local_addr)
    , peer_addr_(peer_addr)
    , high_water_mark_(64 * 1024 * 1024) // 64MB
{
    assert(loop_ != nullptr);
    // LOG_INFO("channel_->setReadCallback()");
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this, _1));
    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(
        std::bind(&TcpConnection::handleError, this));
    LOG_DEBUG("TcpConnection::TcpConection at fd = %d", sockfd_);
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    LOG_DEBUG("TcpConnection::~TcpConnection at fd = %d with state = %s",
                sockfd_,
                stateToString().c_str());
    assert(state_ == kDisconnected);
}

void TcpConnection::send(const std::string& buf) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf.c_str(), buf.size());
        } else {
            loop_->runInLoop(
                std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }
}

void TcpConnection::sendInLoop(const void* data, size_t len) {
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool fault_error = false;
    if (state_ == kDisconnected) {
        LOG_ERROR("disconneced, give up writing");
        return;
    }

    // 如果outputbuffer没有数据，尝试直接写入
    if (!channel_->isWriting() && output_buffer_.readableBytes() == 0) {
        nwrote = sockets::write(channel_->fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && write_complete_callback_) {
                loop_->queueInLoop(
                    std::bind(write_complete_callback_, shared_from_this()));
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_ERROR("TcpConnection::sendInLoop()");
                if (errno == EPIPE || errno == ECONNRESET) {
                    fault_error = true;
                }
            }
        }
    }

    // 还没有写完，需要给 channel 注册 EPOLLOUT事件
    if (!fault_error && remaining > 0) {
        size_t old_len = output_buffer_.readableBytes();
        if (old_len + remaining >= high_water_mark_
            && old_len < high_water_mark_
            && high_water_mark_callback_) {
            loop_->queueInLoop(
                std::bind(high_water_mark_callback_, shared_from_this(), old_len + remaining));
        }
        output_buffer_.append(static_cast<const char *>(data), remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();  // 注册channel写事件，否则poller不会给channel通知POLLOUT
        }
    }
}

void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(
            &TcpConnection::shutdownInLoop, shared_from_this()));
    }
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
}

void TcpConnection::connectEstablished() {
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
    // 执行上层回调
    connection_callback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll(); // 把channel所有感兴趣的事件都删掉
        connection_callback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handleRead(TimeStamp receive_time) {
    LOG_INFO("TcpConnection::handleRead()");
    int save_errno = 0;
    ssize_t n = input_buffer_.readFd(channel_->fd(), &save_errno);
    if (n > 0) {
        message_callback_(shared_from_this(), &input_buffer_, receive_time);
    } else if (n == 0) {
        handleClose();
    } else {
        errno = save_errno;
        LOG_ERROR("TcpConnection::handleRead() read error");
        handleError();
    }
}

void TcpConnection::handleWrite() {
    if (channel_->isWriting()) {
        int save_errno = 0;
        ssize_t n = output_buffer_.writeFd(channel_->fd(), &save_errno);
        if (n > 0) {
            output_buffer_.retrieve(n);
            if(output_buffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (write_complete_callback_) {
                    loop_->queueInLoop(std::bind(
                        write_complete_callback_, shared_from_this()));
                }
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else {
            LOG_ERROR("TcpConnection::handleWrite()");
        }
    } else {
        LOG_ERROR("TcpConnection %d is down, no more writing", channel_->fd());
    }
}

void TcpConnection::handleClose() {
    LOG_INFO("TcpConnection::handleClose() fd = %d, state = %d", channel_->fd(),
                                                                 static_cast<int>(state_));
    setState(kDisconnected);
    channel_->disableAll();
    TcpConnectionPtr guard_this(shared_from_this());
    connection_callback_(guard_this);
    close_callback_(guard_this);
}

void TcpConnection::handleError() {
    int err = sockets::getSocketError(channel_->fd());
    LOG_ERROR("TcpConnection::handleError() error = %s", strerror_tl(err));
}