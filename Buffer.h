#pragma once


# include<cstdlib>
#include <vector>
#include <string>
#include <algorithm>

/**
 * prependable bytes ->  readable bytes -> writable bytes
 * 0             readerIndex (contebr) writerIndex      size  
 */
// 网络库底层的缓冲器类型定义
class Buffer {
 public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  explicit Buffer(size_t initialSize = kInitialSize) 
      : buffer_(kCheapPrepend + initialSize)
      , readerIndex_(kCheapPrepend)
      , writerIndex_(kCheapPrepend) {
  }

  size_t readableBytes() const {
    return writerIndex_ - readerIndex_;
  }

  size_t writableBytes() const {
    return buffer_.size() - readerIndex_;
  }

  size_t prependableBytes() const {
    return readerIndex_;
  }

  // 返回缓冲区中，可读数据的起始地址
  const char* peek() const {
    return begin() + readerIndex_;
  }

  // Onmessage: Buffer -> string
  void retrieve(size_t len) {
    if (len < readableBytes()) {
      readerIndex_ +=len; // 应用只读取了可读缓冲区数据的一部分（len长度）
    } else {  // len == readableBytes()
      retrieveAll();
    }
  }

  void retrieveAll() {
    readerIndex_ = writerIndex_ = kCheapPrepend;
  }
  // 把OnMessage函数上报的Buffer数据，转为string类型的数据返回
  std::string retrieveAllAsString() {
    return retrieveAsString(readableBytes()); // 应用可读取数据的长度
  }

  std::string retrieveAsString(size_t len) {
    std::string result(peek(), len);
    retrieve(len);  // 前一句读出缓冲区可读的数据，这里对缓冲区进行复位操作
    return result;
  }

  // 保证有足够的缓存写空间写数据，否则使用两种方法扩容（直接resize或移动数组填补空闲缓冲）
  void ensureWritableBytes(size_t len) {
    if (writableBytes() < len) {
      makeSpace(len); // 扩容函数
    }
  }
  
  // 把[data, data+len]内存上的数据添加到writbale缓冲区当中
  void append(const char *data, size_t len) {
    ensureWritableBytes(len);
    std::copy(data, data + len, beginWrite());
    writerIndex_ += len;
  }

  // const 与 non const 版本beginWrite函数：确定写缓存地址
  char* beginWrite() {
    return begin() + writerIndex_;
  }
  const char* beginWrite() const {
    return  begin() + writerIndex_;
  }

  // 从fd上读取数据
  ssize_t readFd(int fd, int *saveErrno);
  // 通过fd发送参数
  ssize_t writeFd(int fd, int *saveErrno);
 private:

  char* begin() {
    // 先*再&：从迭代器到字符指针
    return &(*buffer_.begin());
  }

  const char* begin() const {
    return &*buffer_.begin();
  }

  void makeSpace(size_t len) {
    if (writableBytes() + prependableBytes() < kCheapPrepend + len) {
      buffer_.resize(writerIndex_ + len);
    } else {
      size_t readable  = readableBytes();
      std::copy(begin() + readerIndex_, 
                begin() + writerIndex_,
                begin() + kCheapPrepend);
      writerIndex_ = readerIndex_ + readable;
    }
  }

  std::vector<char> buffer_;
  size_t readerIndex_;
  size_t writerIndex_;

};