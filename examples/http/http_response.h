#ifndef MUDUO_EXAMPLES_HTTP_RESPONSE_H
#define MUDUO_EXAMPLES_HTTP_RESPONSE_H

#include <mymuduo/utils/copyable.h> 
#include <mymuduo/utils/buffer.h>

#include <string>
#include <unordered_map>

using muduo::utils::copyable;

class HttpResponse : public copyable
{
public:
    enum HttpStatusCode {
        kUnknown,
        k200OK = 200,
        k301MovePermanently = 301,
        k400BadRequest = 400,
        k404NotFound = 404,
    };

    HttpResponse(bool close) 
        : close_connection_(close)
        , status_code_(kUnknown)
    {
    }

    void setStatusCode(HttpStatusCode code) { status_code_ = code; }
    void setStatusMessage(const std::string& message) { status_message_ = message; }
    void setBody(const std::string& body) { body_ = body; }
    void setCloseConnection(bool should_close) { close_connection_ = should_close; }
    bool closeConnection() const { return close_connection_; }

    void addHeader(const std::string& field, const std::string& value) {
        headers_[field] = value;
    }

    void appendToBuffer(std::string& buf) const;

private:
    HttpStatusCode status_code_;
    std::string status_message_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
    bool close_connection_;
};

#endif