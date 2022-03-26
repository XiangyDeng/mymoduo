#include "EventLoopThreadPool.h"

#include <memory>
#include "EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseloop, const std::string &nameArg)
    : baseLoop_(baseloop)
    , name_(nameArg)
    , started_(false)
    , numThreads_(0)
    , next_(0) {
} 

EventLoopThreadPool::~EventLoopThreadPool() {
  // EventLoopThread 的loop 创建在栈对象上，自动析构
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb) {
  started_ = true;

  for (int i=0; i<numThreads_; ++i) {
    // 初始化各个线程名
    char buf[name_.size()+32];
    snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
    // 单线程初始化
    EventLoopThread *t = new  EventLoopThread(cb, buf);
    // 智能指针管理，并存到vector中
    threads_.push_back(std::unique_ptr<EventLoopThread>(t));
    loops_.push_back(t->startLoop());  // 底层创建线程，绑定一个新的EventLoop，并返回loop的地址

  }

  // 整个服务端只有一个线程，运行着baseloop
  if ( numThreads_ == 0 && cb) {
    cb(baseLoop_);
  }
}

// 如果工作在多线程中，baseloop_（mainReactor）默认以轮询的方式分配channel给subloop（subReactor）
EventLoop* EventLoopThreadPool::getNextLoop() {
  // 从baseLoop开始找可用loop
  EventLoop *loop = baseLoop_;

  if (!loops_.empty()) {  // 通过轮询获取下一个处理事件的loop，如果是空的就只能使用baseLoop
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
    // 若subloop为空，则返回mainloop（战斗至一兵一卒）
    return std::vector<EventLoop*>(1, baseLoop_);
  } else {
    return loops_;
  }
}
