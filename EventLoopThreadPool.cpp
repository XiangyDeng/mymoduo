#include "EventLoopThreadPool.h"

#include <memory>
#include "EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseloop, const std::string &nameArg)
    : baseloop_(baseloop)
    , name_(nameArg)
    , started_(false)
    , numThread_(0)
    , next_(0) {
} 

~EventLoopThreadPool::EventLoopThreadPool() {
  // EventLoopThread 的loop 创建在栈对象上，自动析构
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb) {
  started_ = true;

  for (int i=0; i<numThread_; ++i) {
    char buf[name_.size()+32];
    snprintf(buf, sizeof buf, "%s%d", name_.c__str(), i);
    EventLoopThread *t = new  EventLoopThread(cb, buf);
    loops_.emplace_back(std::unique_ptr<EventLoopThread*>(t));
    loops_.emplace_back(t->startLoop());  // 底层创建线程，绑定一个新的EventLoop，并返回loop的地址

  }

  // 整个服务端只有一个线程，运行着baseloop
  if ( numThreads_ == 0 && cb) {
    cb(baseloop_);
  }
}

// 如果工作在多线程中，baseloop_（mainReactor）默认以轮询的方式分配channel给subloop（subReactor）
EventLoop* EventLoopThreadPool::getNextLoop() {
  EventLoop *loop = baseLoop_;

  if (!loops_.empty()) {  // 通过轮询获取下一个处理事件的loop
    loop = loops_[next_];
    ++next_;
    if (next_ >= loops_.size()) {
      next_ = 0;
    }
  }

  return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
  if (loops_.empty()) {
    return std::vetcor<EventLoop*>(1, baseloop_);
  } else {
    return loops_;
  }
}
