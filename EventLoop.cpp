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
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
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
    if (t_loopInThisThread) {
      LOG_FATAL("Another EventLoop %p exists in this thread %d \n", t_loopInThisThread, threadId_);
    } else {
      t_loopInThisThread = this;
    }

    // 设置wakeupfd的事件类型以及发生事件后的回调函数
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // 每一个EventLoop都将监听wakeupChannel的EPOLLIN读事件
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;

}

// 开启事件循环：调用底层Poller，开启事件分发器
// 1.loop在自己的线程中调用quit. 2.在非loop的线程中调用loop的quit
void EventLoop::loop() {
  quit_ = true;

  // 如果是在其他线程中调用quit，例如在一个subloop(worker)中调用了mainloop的quit
  if (!isInLoopThread()) {
    wakeup();
  }

}
// 退出事件循环
void EventLoop::quit() {
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

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = read(wakeupFd_, &one, sizeof one);
  if(n != sizeof one) {
    LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead of 8", n);
  }
}


