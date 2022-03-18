#include "Thread.h"

#include "CurrentThread.h"

std::atomic_int Thread::numCreated_ = 0;

Thread::Thread(ThreadFunc func, const std::string &name)
    : started_(false)
    , joined_(false)
    , tid_(0)
    , name_(name)
    , func_(func) {
      setDefaultName();

}
~Thread() {
  if (started_ && !jioned_) {
    thread_->detech();  // thread类提供的设置分离线程的方法：守护线程（pthread_detech）
  }
}

void Thread::start() {
  started_ = true;
  thread_ = std::shared_ptr<std::thread>(new std::thread([&](){

  }));
}
void Thread::join() {

}

void Thread::setDefaultName() {
  int num = ++numCreated_;
  if (name_.empty()) {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread%d", num);
    name_ = buf;
  }
}