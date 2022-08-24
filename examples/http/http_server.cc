#include "http_server.h"
#include "http_request.h"
#include "http_response.h"
#include "http_context.h"
#include <mymuduo/utils/logger.h>

// #include <functional>
using namespace std::placeholders;

void defaultHttpCallback(const HttpRequest& request, HttpResponse* response) {
    response->setStatusCode(HttpResponse::k404NotFound);
    response->setStatusMessage("Not Found");
    response->setCloseConnection(true);
}

HttpServer::HttpServer(EventLoop* loop, const InetAddress& local_addr) 
    : server_(loop, local_addr, "HttpServer")
    , http_callback_(defaultHttpCallback)
{
    server_.setConnectionCallback(
        std::bind(&HttpServer::onConnection, this, _1)
    );
    server_.setMessageCallback(
        std::bind(&HttpServer::onMessage, this, _1, _2, _3)
    );
}

void HttpServer::onConnection(const TcpConnectionPtr& conn) {
    // LOG_INFO("HttpServer : %s -> %s is %s", conn->peerAddress().toIpPort().c_str(),
    //                                         conn->localAddress().toIpPort().c_str(),
    //                                         (conn->connected()) ? "UP" : "DOWN");
    if (conn->connected()) {
        conn->setContext(HttpContext());
    }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn,
                           muduo::utils::Buffer* buf,
                           muduo::utils::TimeStamp receive_time) {
    // LOG_INFO("HttpServer::onMessage");

    HttpContext* context = std::any_cast<HttpContext>(conn->getMutableContext());
    if (!context->parseRequest(buf, receive_time)) {
        conn->shutdown();
    }
    if (context->allParsed()) {
        onRequest(conn, context->request());
        context->reset();
    }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& request) {
    // LOG_INFO("HttpServer::onRequest");

    const std::string& connection = request.getHeader("Connection");
    bool should_close = connection == "close" || 
        (request.version() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    
    HttpResponse response(should_close);
    http_callback_(request, &response);

    std::string buf;
    response.appendToBuffer(buf);
    conn->send(buf.c_str());

    if (response.closeConnection()) {
        conn->shutdown();
    }
}

void HttpServer::setThreadNum(int thread_num) {
    server_.setThreadNum(thread_num);
}

void HttpServer::start() {
    server_.start();
}