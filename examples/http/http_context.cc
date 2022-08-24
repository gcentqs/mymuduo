#include "http_context.h"
#include <mymuduo/utils/buffer.h>
#include <mymuduo/utils/logger.h>

#include <algorithm>


bool HttpContext::parseRequest(muduo::utils::Buffer* buf, muduo::utils::TimeStamp receive_time) {
    // LOG_INFO("HttpContext::parseRequest");
    bool ok = true;
    bool is_end = false;
    // LOG_INFO("0, state = %d", state_);

    while (!is_end) {
        if (state_ == kExpectRequestLine) {
            const char* crlf = buf->findCRLF();
            if (crlf) {
                ok = parseRequestLine(buf->peek(), crlf);
                if (ok) {
                    request_.setReceiveTime(receive_time);
                    buf->retrieveUntil(crlf + 2);
                    state_ = kExpectHeaders;
                } else {
                    is_end = true;
                }
            } else {
                is_end = true;
            }
        } else if (state_ == kExpectHeaders) {
            const char* crlf = buf->findCRLF();
            if (crlf) {
                const char* colon = std::find(buf->peek(), crlf, ':');
                if (colon != crlf) {
                    request_.addHeader(buf->peek(), colon, crlf);
                } else {
                    // empty line, end of header
                    state_ = kAllParsed;
                    is_end = true;
                }
                buf->retrieveUntil(crlf + 2);
            } else {
                is_end = true;
            }
        } else if (state_ == kExpectBody) {
            // TODO
        }
    }
    return ok;
}

// 解析请求行
bool HttpContext::parseRequestLine(const char* begin, const char* end) {
    bool success = false;
    const char* cur = begin;

    // method
    const char* space = std::find(cur, end, ' ');
    if (space == end) {
        return success;
    }
    request_.setMethod(cur, space);
    cur = space + 1;

    // url
    space = std::find(cur, end, ' ');
    if (space == end) {
        return success;
    }
    const char* question = std::find(cur, space, '?');
    if (question != space) {
        request_.setPath(cur, question);
        request_.setQuery(question, space);
    } else {
        request_.setPath(cur, space);
    }
    cur = space + 1;

    // version
    std::string v(cur, end);
    if (v.size() != 8 && v.substr(0, 7) != "HTTP/1.") {
        return success;
    }

    success = true;
    if (v.back() == '0') {
        request_.setVersion(HttpRequest::kHttp10);
    } else if (v.back() == '1') {
        request_.setVersion(HttpRequest::kHttp11);
    } else {
        success = false;
    }
    return success;
}
