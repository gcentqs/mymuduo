#include "buffer.h"
#include "logger.h"

#include <sys/uio.h>
#include <unistd.h>

using namespace muduo::utils;

/**
 * 从fd读取数据是 LT模式
 * 从socket读到缓冲区是用readv先读到buffer中  
 * buffer空间不够时，会读入到栈extrabuf中，然后以append方式写入buffer 
 * 既避免系统调用导致的开销(尽可能一次性读完)，又不影响数据的接收
 **/

// 从socket写入buffer
ssize_t Buffer::readFd(int fd, int* save_errno) {
    char extra_buf[65536] = {0};
    /**
     * struct iovec {
     *     ptr_t iov_base;
     *     size_t iov_len;
     * }
     **/
    struct iovec vec[2];
    const size_t writable = writableBytes();

    // 第一块指向buffer中可写部分
    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writable;
    // 第二块指向栈空间
    vec[1].iov_base = extra_buf;
    vec[1].iov_len = sizeof(extra_buf);

    const int iovcnt = (writable < sizeof(extra_buf)) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);

    if (n < 0) {
        *save_errno = errno;
    } else if (n < writable) {
        writer_index_ += n;
    } else {
        writer_index_ = buffer_.size();
        append(extra_buf, n - writable);
    }
    return n;
}

// 从buffer读到socket中（内核缓冲区）
ssize_t Buffer::writeFd(int fd, int* save_errno) {
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0) {
        *save_errno = errno;
    }
    return n;
}