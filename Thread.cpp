#include "Thread.h"

#include <semaphore.h>
#include "CurrentThread.h"

std::atomic_int Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, const std::string &name)
    : started_(false)
    , joined_(false)
    , tid_(0)
    , name_(name)
    , func_(func) {
      setDefaultName();
}

Thread::~Thread() {   // 一个Thread对象记录的就是一个新线程的详细信息
  if (started_ && !joined_) {
    thread_->detach();  // thread类提供的设置分离线程的方法：守护线程（pthread_detach）
  }
}

void Thread::start() {
  started_ = true;
  sem_t sem;
  sem_init(&sem, false, 0);
  // 以引用的方式接收Thread对象，开启线程
  thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
    // 获取线程的tid
    tid_ = CurrentThread::tid();
    sem_post(&sem);
    // 开启一个新线程，专门执行该线程的函数
    func_(); 
  }));

  // 这里必须等待获取上面新创建的线程的tid值
  sem_wait(&sem);
}
void Thread::join() {
  joined_ = true;
  thread_->join();
}

void Thread::setDefaultName() {
  int num = ++numCreated_;
  if (name_.empty()) {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread%d", num);
    name_ = buf;
  }
}