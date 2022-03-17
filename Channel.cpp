#include"Channel.h"

#include<sys/epoll.h>
#include"EventLoop.h"
#include"Logger.h"

 // 当前状态描述初始化
const int Channel::kNoneEvent = 0;;  
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd) 
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false) {
    }

Channel::~Channel() { 
}

// 在其他模块触发
void Channel::tie(const std::shared_ptr<void>& obj) {
  tie_ = obj; // weak观察强智能指针
  tied_ = true;
}

/**
 * @brief 当改变Channel所表示fd的events事件后，update负责在poller（epoll）里
 *        改变相应事件的epoll_ctl：EventLoop => ChannelList + Poller，即
 *         ChannelList 通知 EventLoop => EventLoop 调用 Poller 做出调整
 */
void Channel::update() {
  // 通过Channel所属的EventLoop，调用Poller的相应方法注册fd的events事件
  // add code...
  // loop_->updateChannel(this);
}

// 在Channel所属的EventLoop中，把当前Channel删除掉
void Channel::remove() {
  // add code...
  // loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime) {
  if (tied_) {
    std::shared_ptr<void> guard = tie_.lock();
    if (guard) {
      handleEventWithGuard(receiveTime);
    }
  }
  else {
    handleEventWithGuard(receiveTime);
  }
}

// 根据Poller通知的Channel发生的具体事件，由Channel负责调用具体的回调操作
void Channel::handleEventWithGuard(Timestamp receiveTime) {
  LOG_INFO("Channel handleEvent revents: %d\n", revents_);

  if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
    if(closeCallback_) {
      closeCallback_();
    }
  }

  if (revents_ & EPOLLERR) {
    if (errorCallback_) {
      errorCallback_();
    }
  }

  if (revents_ & (EPOLLIN | EPOLLPRI)) {
    if (readCallback_) {
      readCallback_(receiveTime);
    }
  }  

  if (revents_ & EPOLLOUT) {
    if (writeCallback_) {
      writeCallback_();
    }
  }
}

