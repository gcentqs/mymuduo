#ifndef MUDUO_EXAMPLES_HTTP_REQUEST_H
#define MUDUO_EXAMPLES_HTTP_REQUEST_H

#include <mymuduo/utils/timestamp.h>

#include <assert.h>

#include <map>
#include <string>

using muduo::utils::TimeStamp;

class HttpRequest
{
public:
    enum Method {
        kInvalid,
        kGet,
        kPost,
        kHead,
        kPut,
        kDelete,
    };

    enum Version {
        kUnknown, 
        kHttp10, 
        kHttp11,   
    };

    HttpRequest()
        : method_(kInvalid)
        , version_(kUnknown)
    {
    }
    ~HttpRequest() = default;

    void setVersion(Version version) { version_ = version; }
    const Version& version() const { return version_; }

    const Method& method() const { return method_; }
    void setMethod(const std::string& m) {
        if (m == "GET") {
            method_ = kGet;
        } else if (m == "POST") {
            method_ = kPost;
        } else if (m == "HEAD") {
            method_ = kHead;
        } else if (m == "PUT") {
            method_ = kPut;
        } else if (m == "DELETE") {
            method_ = kDelete;
        } else {
            method_ = kInvalid;
        }
    }
    void setMethod(const char* begin, const char* end) {
        assert(method_ == kInvalid);
        std::string method{begin, end};
        setMethod(method);
    }

    std::string methodToString() const {
        std::string res;
        switch (method_) {
            case kGet: res = "GET"; break;
            case kPost: res = "POST"; break;
            case kHead: res = "HEAD"; break;
            case kPut: res = "PUT"; break;
            case kDelete: res = "DELETE"; break; 
            default: res = "UNKNOWN"; break;
        }
        return res;
    }

    const std::string& path() const { return path_; }
    void setPath(const std::string& path) { path_ = path; }
    void setPath(const char* begin, const char* end) { path_.assign(begin, end); }

    const std::string& query() const { return query_; }
    void setQuery(const std::string& query) { query_ = query; }
    void setQuery(const char* begin, const char* end) { query_.assign(begin, end); }

    const TimeStamp& receiveTime() const { return receive_time_; }
    void setReceiveTime(TimeStamp t) { receive_time_ = t; }

    const std::map<std::string, std::string>& headers() const { return headers_; }
    std::string getHeader(const std::string& field) const {
        auto it = headers_.find(field);
        if (it == headers_.end()) {
            return {};
        }
        return it->second;
    }

    void addHeader(const std::string& field, const std::string& value) {
        headers_[field] = value;
    }
    void addHeader(const char* begin, const char* colon, const char* end) {
        std::string field{begin, colon};
        while (colon < end && (*colon) == ' ') {
            ++colon;
        }
        std::string value{colon, end};
        while (!value.empty() && value.back() == ' ') {
            value.pop_back();
        }
        headers_[field] = value;
    }

    void swap(HttpRequest& rhs) {
        std::swap(method_, rhs.method_);
        std::swap(version_, rhs.version_);
        path_.swap(rhs.path_);
        query_.swap(rhs.query_);
        receive_time_.swap(rhs.receive_time_);
        headers_.swap(rhs.headers_);
    }

private:
    Method method_;
    Version version_;
    std::string path_;
    std::string query_;
    TimeStamp receive_time_;
    std::map<std::string, std::string> headers_;
};

#endif