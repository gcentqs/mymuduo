#include "event_loop.h"
#include "utils/socketops.h"
#include "tcp_server.h"


using namespace muduo;
using namespace muduo::utils;
using namespace std::placeholders;

static EventLoop* checkLoopNotNull(EventLoop* loop) {
    if (loop == nullptr) {
        LOG_FATAL("%s:%s:%d main loop is null", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}


TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& listen_addr,
                     const std::string& name_arg,
                     Option opt)
    : loop_(checkLoopNotNull(loop))
    , ip_port_(listen_addr.toIpPort())
    , name_(name_arg)
    , acceptor_(new Acceptor(loop, listen_addr, opt == kReusePort))
    , thread_pool_(new EventLoopThreadPool(loop, name_))
    , connection_callback_(defaultConnectionCallback)
    , message_callback_(defaultMessageCallback)
    , next_conn_id_(1)
{
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, _1, _2)
    );
}

TcpServer::~TcpServer() {
}

void TcpServer::setThreadNum(int num_threads) {
    thread_pool_->setThreadNum(num_threads);
}    

void TcpServer::start() {
    if (started_++ == 0) {    // 防止一个TcpServer对象被start多次
        thread_pool_->start();
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}


void TcpServer::newConnection(int sockfd, const InetAddress& peer_addr) {
    loop_->assertInLoopThread();
    EventLoop *io_loop = thread_pool_->getNextLoop(); 

    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", ip_port_.c_str(), next_conn_id_++);
    std::string conn_name = name_ + buf;
    LOG_INFO("TcpServer::newConnection: [%s accept new connection %s] from %s", 
             name_.c_str(),
             conn_name.c_str(),
             peer_addr.toIpPort());
    InetAddress local_addr(sockets::getLocalAddress(sockfd));
    TcpConnectionPtr conn(new TcpConnection(io_loop,
                                         conn_name,
                                         sockfd,
                                         local_addr,
                                         peer_addr));
    connections_[conn_name] = conn;

    conn->setConnectionCallback(connection_callback_);
    conn->setMessageCallback(message_callback_);
    conn->setWriteCompleteCallback(write_complete_callback_);
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, _1)
    );
    io_loop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    loop_->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    loop_->assertInLoopThread();
    LOG_INFO("TcpServer::removeConnectionInLoop: [%s removed connection %s] from %s",
             conn->name().c_str(),
             name_.c_str(),
             conn->peerAddress().toIpPort().c_str());
    
    size_t n = connections_.erase(conn->name());
    assert(n == 1);
    EventLoop* io_loop = conn->getEventLoop();
    io_loop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn));
}