#pragma once 

#include <unistd.h>
#include <functional>
#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include "noncopyable.h"

class Thread : noncopyable {
 public:
  using ThreadFunc = std::function<void()>;

  explicit Thread(ThreadFunc, const std::string &name = std::string());
  ~Thread();

  void start();
  void join();

  bool started() const { return started_; }
  pid_t tid() const { return tid_; }
  const std::string& name() const { return name_; }
  static int numCreated() { return numCreated_; }

 private:
  void setDefaultName();
  
  bool started_;
  bool joined_;   // 当前线程等待其他线程运行完毕再进行往下执行
  // 需要使用智能指针封装，因为直接声明thread会直接开启线程
  std::shared_ptr<std::thread> thread_;
  pid_t tid_;
  ThreadFunc func_;
  std::string name_;
  static std::atomic_int numCreated_;



};