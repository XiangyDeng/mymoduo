#include "Buffer.h"

#include <sys/uio.h>
#include <errno.h>
#include <unistd.h>

/**
 * @brief 从fd上读取数据， Poller为LT模式， Buffer缓冲区是有大小的！但是从fd上都要去数据时
 *        是不知道tcp数据流的最终大小（区别于udp的数据报）
 * 
 * @param fd 
 * @param savedErrno 
 * @return ssize_t 
 */
ssize_t Buffer::readFd(int fd, int* savedErrno) {
  char extrabuf[65536] = {0}; // 栈上的内存空间(64k)：分配速度快，离开作用域自动回收
  struct iovec vec[2];

  const size_t writable = writableBytes();  // 这是Buffer底层缓冲区剩余的科协空间大小
  vec[0].iov_base = begin() + writerIndex_;
  vec[0].iov_len = writable;

  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;

  const int iocnt = (writable < sizeof extrabuf) ? 2 : 1;
  const ssize_t n = ::readv(fd, vec, iocnt);
  if (n < 0) {
    *savedErrno = errno;
  } else if (n <= n) {
    writerIndex_ += n;
  } else { // extrabuf里面也写入了数据
    writerIndex_ = buffer_.size();
    append(extrabuf, n - writable); // 从writerIndex_开始写大小为n - writable的数据
  }
  return n;
}

ssize_t Buffer::writeFd(int fd, int *saveErrno) {
  ssize_t n = ::write(fd, peek(), readableBytes());
  if (n < 0) {
    *saveErrno = errno;
  } 
  return n;
}
