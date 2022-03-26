#include"EventLoop.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <memory>

#include "Logger.h"
#include "Channel.h"
#include "Poller.h"

// 防止一个线程创建多个EventLoop : one thread one loop thread_local
__thread EventLoop *t_loopInThisThread = nullptr;

// 定义默认的Poller IO复用接口的超时时间
const int kPollTimeMs = 10000;

// 创建wakeupfd，用来notify唤醒subReactor处理信赖的channel
int createEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC); // 非阻塞 + fork不遗传父类
  if (evtfd < 0) {
    LOG_FATAL("eventfd error:%d \n",errno);
  }
  return evtfd; 
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      currentActiveChannel_(nullptr) {
    
    LOG_DEBUG("EventLoop created %p in thread %d \n", this, threadId_);
    // one thread one loop
    if (t_loopInThisThread) {
      LOG_FATAL("Another EventLoop %p exists in this thread %d \n", t_loopInThisThread, threadId_);
    } else {
      t_loopInThisThread = this;
    }

    // 设置wakeupfd的事件类型 以及 发生事件后的回调函数
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // 每一个EventLoop都将监听wakeupChannel的EPOLLIN读事件
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    wakeupChannel_->disableAll(); 
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr; // 

}

// 退出事件循环
/**
 * 退出事件循环：1.loop在自己的线程中调用quit 2. 在非loop的线程中，调用loop的quit 
 *                       mainLoop
 *                                    
 *                              no ============ 生产者-消费者的线程安全的队列(改进点)
 * 
 *        subLoop1      subLoop2        subLoop3
 */
void EventLoop::quit() {
  quit_ = true;

  // 如果是在其他线程中调用quit，例如在一个subloop(worker)中调用了mainloop(IO)的quit
  if (!isInLoopThread()) {
    wakeup();
  }
}

// 开启事件循环：调用底层Poller，开启事件分发器
void EventLoop::loop() {
  looping_ = true;
  quit_ = false;

  LOG_INFO("EventLoop %p start looping \n", this);

  while(!quit_) {
    activeChannels_.clear();
    // 监听两个fd：clientfd 与 wakeupfd
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);

    for (Channel *channel : activeChannels_) {

      // Poller监听哪些channel发生事件了， 并上报EventLoop，通知channel处理相应的事件
      channel->handleEvent(pollReturnTime_);
    }

    // 执行当前EventLoop事件循环需要处理的回调操作
    /**
     *  IO线程 mainLoop (accept) <= fd(Channel) subloop
     *  mainLoop 事先注册一个回调（需要subloop来执行）
     *  wakeup subloop后，执行下面的函数，执行之前mainloop注册的cb
     */
    doPendingFunctors();  
  }

  LOG_INFO("EventLoop %p stop looping. \n", this);
  looping_ = false;
}

// 在当前loop中执行cb
void EventLoop::runInLoop(Functor cb) {
  if (isInLoopThread()) { // 在当前的loop线程中，执行cb；
    cb();
  } else {  //* 在非当前loop线程中执行cb，就需要唤醒loop所在线程，执行cb
    queueInLoop(cb);
  }
}
//? 把cb放入到队列（数组？）中，唤醒loop所在线程，执行cb
void EventLoop::queueInLoop(Functor cb) {
  {
    std::unique_lock<std::mutex> lock(mutex_);
    pendingFunctors_.emplace_back(cb);
  }
  
  // 唤醒相应的，需要执行上面回调操作的loop的线程
  // callingPendingFunctors_表示当前loop正在执行回调，但是loop又有了新的回调
  if (!isInLoopThread() || callingPendingFunctors_) { 
    wakeup();
  }
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = read(wakeupFd_, &one, sizeof one);
  if(n != sizeof one) {
    LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead of 8", n);
  }
}

// 用来唤醒loop所在线程:向wakeupfd_写一个数据, wakeupChannel就发生读事件，当前loop线程就会被唤醒
void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = write(wakeupFd_ , &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8 \n", n);
  }
}

// EventLoop的方法 => Poller的方法
void EventLoop::updateChannel(Channel *channel) {
  poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
  poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
  poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;  // 交给栈对象处理：自动清除
  callingPendingFunctors_ = true;

  {
    std::unique_lock<std::mutex> lock(mutex_);
    functors.swap(pendingFunctors_);  // pendingFunctors_清零，避免阻塞
  }

  for (const Functor &functor : functors) {
    functor();
  }

  callingPendingFunctors_ = false;
}


