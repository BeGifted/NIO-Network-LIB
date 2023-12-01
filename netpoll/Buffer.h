#pragma once

#include <vector>
#include <string>
#include <algorithm>

class Buffer {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        :buffer_(kCheapPrepend + initialSize)
        ,readerIndex_(kCheapPrepend)
        ,writerIndex_(kCheapPrepend) {

    }

    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    size_t writableBytes() const { return buffer_.size() - writerIndex_; }
    size_t prependableBytes() const { return readerIndex_; }
    //返回buffer中可读数据的起始地址
    const char* peek() const { return begin() + readerIndex_; }

    void retrieve(size_t len) {
        if (len < readableBytes()) {
            readerIndex_ += len;  //只读取了可读buffer一部分
        } else {
            retrieveAll();
        }
    }

    void retrieveAll() {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    //把onmessage上报的buffer，转成string
    std::string retrieveAllAsString() {
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len) {
        std::string result(peek(), len);
        retrieve(len);  //把可读数据读取出来，这里对buffer进行复位操作
        return result;
    }

    void ensureWritableBytes(size_t len) {
        if (writableBytes() < len) {
            makeSpace(len);
        }
    }

    void append(const char* data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        writerIndex_ += len;
    }

    char* beginWrite() { return begin() + writerIndex_; }
    const char* beginWrite() const { return begin() + writerIndex_; }

    //从fd上读取数据
    ssize_t readFd(int fd, int* saveErrno);

    ssize_t writeFd(int fd, int* saveErrno);

private:
    char* begin() { return &*buffer_.begin(); }
    const char* begin() const { return &*buffer_.begin(); }
    
    void makeSpace(size_t len) {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
            buffer_.resize(writerIndex_ + len);
        } else {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writerIndex_, 
                begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

};