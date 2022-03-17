#include "Poller.h"
#include <stdlib.h>
#include "EventLoop.h"


// 实现要点：避免继承体系出错！
Poller* Poller::newDefaultPoller(EventLoop *loop) {
  if (::getenv("MUDUO_USE_POLL")) {
    return nullptr; // 生成poll的实例
  } else {
    return nullptr; // 生成epoll的实例
  } 
}