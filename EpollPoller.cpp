#include "EpollPoller.h"
#include "Logger.h"

#include <errno.h>
#include <unistd.h>

const int kNew = -1;    
const int kAdded = 1;   // channel从epoll中添加
const int kDeleted = 2; // 删除

EpollPoller::EpollPoller(EventLoop *loop) 
    : Poller(loop),
      epollfd_(::epoll_create(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
      if(epollfd_ < 0) {
        LOG_FATAL("epoll_create error: ", errno);
      }
    
    }

EpollPoller::~EpollPoller() {
  ::close(epollfd_);
}
