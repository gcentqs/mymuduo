#ifndef MUDUO_UTILS_BUFFER_H
#define MUDUO_UTILS_BUFFER_H

#include "copyable.h"
#include "logger.h"

#include <algorithm>
#include <string>
#include <vector>

#include <assert.h>
#include <stdlib.h>

namespace muduo
{

namespace utils
{
/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

class Buffer : public copyable
{
public:    
    static const size_t kCheapPrepend = 8;  // 头部长度
    static const size_t kInitialSize = 1024;    // buf初始长度
    static const char kCRLF[];
public:
    explicit Buffer(size_t inital_size = kInitialSize)
        : buffer_(kInitialSize)
        , reader_index_(kCheapPrepend)
        , writer_index_(kCheapPrepend) 
    {
#ifdef MUDEBUG
        assert(readableBytes() == 0);
        assert(writableBytes() == kInitialSize);
        assert(prependableBytes() == kCheapPrepend;)
#endif
    }
    
    void swap(Buffer& rhs);

    size_t readableBytes() const { return writer_index_ - reader_index_; }
    size_t writableBytes() const { return buffer_.size() - writer_index_; }
    size_t prependableBytes() const { return reader_index_; }

    const char* peek() const { return begin() + reader_index_; }

    void retrieve(size_t len) {
        assert(len <= readableBytes());
        if (len < readableBytes()) {
            reader_index_ += len;
        } else {
            retrieveAll();
        }
    }
    void retrieveUntil(const char* end) {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }
    void retrieveAll() {
        reader_index_ = kCheapPrepend;
        writer_index_ = kCheapPrepend;
    }

    std::string retriveAllAsString() {
        return retriveAsString(readableBytes());
    }
    std::string retriveAsString(size_t len) {
        assert(len <= readableBytes());
        std::string res(peek(), len);
        retrieve(len);
        return res;
    }

    const char* findCRLF() const {
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? nullptr : crlf;
    }

    void append(const char* data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
    }

    void append(const std::string& data) {
        append(data.c_str(), data.size());
    }

    void ensureWritableBytes(size_t len) {
        if (writableBytes() < len) {
            makeSpace(len);
        }
        assert(len <= writableBytes());
    }


    char* beginWrite() { return begin() + writer_index_; }
    const char* beginWrite() const { return begin() + writer_index_; }

    ssize_t readFd(int fd, int* save_errno);
    ssize_t writeFd(int fd, int* save_errno);

private:
    char *begin() { return buffer_.data(); }
    const char* begin() const { return buffer_.data(); }

    void makeSpace(size_t len) {
        // buffer.size() - write_index + read_index <==> len >= read_index - kCheapPrepend
        /**
         * | kCheapPrepend |xxx| reader | writer |                     // xxx标示reader中已读的部分
         * | kCheapPrepend | reader ｜          len          |
         **/
        if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
            buffer_.resize(writer_index_ + len);
        } else {
            size_t readable = readableBytes();
            std::copy(begin() + reader_index_,
                      begin() + writer_index_,
                      begin() + kCheapPrepend);
            reader_index_ = kCheapPrepend;
            writer_index_ = reader_index_ + readable;
        }

    }

private:
    std::vector<char> buffer_;
    size_t reader_index_;
    size_t writer_index_;
};

}

}

#endif