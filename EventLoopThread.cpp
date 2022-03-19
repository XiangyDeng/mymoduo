#include "EventLoopThread.h"

#include <memory>
#include "EventLoop.h"


EventLoopThread::EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(), const std::string &name = std::string())
    : loop_(nullptr)
    , exiting_(false)
    , thread(std::bind(&EventLoopThread::threadFunc,this), name)
    , mutex_()
    , cond_()
    , callback_(cb) {
}


~EventLoopThread::EventLoopThread() {
  exiting_ = true;
  if (loop_ != nullptr) {
    loop_->quit();
    thread_.join();
  }
}

// 新线程会自动创建EventLoop，并返回
EventLoopThread* EventLoopThread::startloop() {
  thread_.start();  // 启动底层新线程

  EventLoop *loop = nullptr;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    while (loop_ == nullptr) {
      cond_.wait(lock);
    }
    loop = loop_;
  }
  return loop;
}

// 是在单独的新线程里运行的
void EventLoopThread::threadFunc() {
  // 重要：创建一个独立的eventloop，和上面的线程是一一对应的，one loop per thread
  EventLoop loop; 

  if (callback_) {
    callback_(&loop);
  }

  {
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = loop;
    cond_.notify_one();
  }

  loop.loop();  // EventLoop loop => Poller.poll
  std::unique_lock<std::mutex> lock(mutex_);
  loop_ = nullptr;
}