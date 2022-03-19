#pragma 

#include <functional>
#include <string>
#include <vector>
#include <memory>
#include "noncopyable.h"

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;

  EventLoopThreadPool(EventLoop *baseloop, const std::string &nameArg);
  ~EventLoopThreadPool();

  void setThreadNum(int numThreads) { numThreads_ = numThreads; }

  void start(const ThreadInitCallback &cb = ThreadInitCallback());

  // 如果工作在多线程中，baseloop_（mainReactor）默认以轮询的方式分配channel给subloop（subReactor）
  EventLoop* getNextLoop();

  std::vector<EventLoop*> getAllLoops();

  bool started() const { return started_; }
  const std::string name() const { return nume_; }

 private:
  EventLoop *baseLoop_; // EventLoop loop: 最初始，由用户创建的loop
  std::string name_;
  bool started_;
  int numThread_;
  int next_;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop*> loops_;
};  