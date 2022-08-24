#ifndef MUDUO_EXAMPLES_HTTP_CONTEXT_H
#define MUDUO_EXAMPLES_HTTP_CONTEXT_H

#include <mymuduo/utils/copyable.h> 
#include <mymuduo/utils/timestamp.h> 
#include <mymuduo/utils/buffer.h>
#include "http_request.h"

using muduo::utils::copyable;


class HttpContext : public copyable
{
public:
    enum HttpRequestParseState {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kAllParsed,
    };

    HttpContext() : state_(kExpectRequestLine)
    {
    }
    ~HttpContext() = default;

    bool parseRequest(muduo::utils::Buffer* buf, muduo::utils::TimeStamp receive_time);

    bool allParsed() { return state_ == kAllParsed; }

    void reset() {
        state_ = kExpectRequestLine;
        HttpRequest dummy;
        request_.swap(dummy);
    }

    HttpRequest& request() { return request_; }
    const HttpRequest& request() const { return request_; } 

private:
    bool parseRequestLine(const char* start, const char* end);

    HttpRequestParseState state_;
    HttpRequest request_;
};


#endif