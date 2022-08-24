#ifndef MUDUO_EXAMPLES_HTTP_SERVER_H
#define MUDUO_EXAMPLES_HTTP_SERVER_H

#include <mymuduo/tcp_server.h> 

class HttpRequest;
class HttpResponse;

class HttpServer : public noncopyable
{
public:
    typedef std::function<void(const HttpRequest&, HttpResponse*)> HttpCallback;

    HttpServer(EventLoop* loop, const InetAddress& local_addr);
    ~HttpServer() = default;

    void setThreadNum(int thread_num);
    void start();

    void setHttpCallback(const HttpCallback& cb) { http_callback_ = std::move(cb); }

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const muduo::utils::TcpConnectionPtr& conn,
                   muduo::utils::Buffer* buf,
                   muduo::utils::TimeStamp receive_time);
    void onRequest(const TcpConnectionPtr& conn, const HttpRequest& request);

private:
    TcpServer server_;
    HttpCallback http_callback_;
};

#endif