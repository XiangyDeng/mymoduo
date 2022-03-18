#include"EventLoop.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "Logger.h"


// 防止一个线程创建多个EventLoop : one thread one loop thread_local
__thread EventLoop *t_loopInisThread = nullptr;

// 定义默认的Poller IO复用接口的超时时间
const int kPollTimeMs = 10000;

// 创建wakeupfd，用来notify唤醒subReactor处理信赖的channel
int createEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG_FATAL("eventfd error:%d \n",errno);
  }
  return evtfd; 
}

EventLoop::EventLoop()
    : looping_

