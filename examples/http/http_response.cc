#include "http_response.h"
#include <mymuduo/utils/buffer.h>


void HttpResponse::appendToBuffer(std::string& buf) const {
    buf = "HTTP/1.1 ";
    buf += std::to_string(status_code_);
    buf += "\r\n";

    if (close_connection_) {
        buf += "Connection: close\r\n";
    } else {
        buf += "Content-Length: ";
        buf += std::to_string(body_.size());
        buf += "\r\n";
        buf += "Connection: Keep-Alive\r\n";
    }

    for (const auto& [field, value] : headers_) {
        buf += (field + ": " + value);
        buf += "\r\n";
    }

    buf += "\r\n";
    buf += body_;
}