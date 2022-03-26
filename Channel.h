#pragma once

#include<memory>
#include<functional>
#include"noncopyable.h"
#include"Timestamp.h"


class EventLoop;

/**
 * @brief Channel类理解为通道，封装了sockfd与其感兴趣的event事件，如EPOLLIN、EPOLLOUT事件
 *        还绑定了poller返回的具体事件，理清楚 EventLoop Channel Poller之间的关系，对应
 *        与Reactor模型上的多路分发器Demultiplex
 */
class Channel : noncopyable {
 public:
  using EventCallback = std::function<void()>;
  using ReadEventCallback = std::function<void(Timestamp)>;

  Channel(EventLoop *loop, int fd);
  ~Channel();

  // fd得到poller通知以后，调用事件相应的回调函数。
  void handleEvent(Timestamp receiveTime);

  // 设置回调函数对象
  void setReadCallback(ReadEventCallback cb) {
    readCallback_ = std::move(cb);
  }
  void setWriteCallback(EventCallback cb) {
    writeCallback_ = std::move(cb);
  }
  void setCloseCallback(EventCallback cb) {
    closeCallback_ = std::move(cb);
  }
  void setErrorCallback(EventCallback cb) {
    errorCallback_ = std::move(cb);
  }

  // 防止当Channel被手动remove掉，Channel还在执行回调操作
  void tie(const std::shared_ptr<void>&);

  int fd() const {
    return fd_;
  }
  int events() const {
    return events_;
  }
  void set_revents(int revt) {
    revents_ = revt;  // used by pollers
  }


  // ! 设置fd相应的事件状态: channel中可更新Poller状态
  void enableReading() {
    events_ |= kReadEvent;
    update();
  }
  void disableReading() {
    events_ &= kReadEvent;
    update();
  }
  void enableWriting() {
    events_ |= kWriteEvent;
    update();
  }
  void disableWriting() {
    events_ &= kWriteEvent;
    update();
  }
  void disableAll() {
    events_ = kNoneEvent;
    update();
  }

  // 返回fd当前的事件状态
  bool isNoneEvent() const {
    return events_ == kNoneEvent;
  }
  bool isReading() const {
    return events_ & kReadEvent;
  }
  bool isWriting() const {
    return events_ & kWriteEvent;
  }

  int index() const {
    return index_;
  }
  void set_index(int idx) {
    index_ = idx;
  }

  // one loop per thread
  EventLoop* ownerLoop() const {
    return loop_;
  }
  void remove();

 private:

  void update(); 
  void handleEventWithGuard(Timestamp receiveTime);

  // 当前状态描述
  static const int kNoneEvent;  
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop *loop_; // 事件循环
  const int fd_;    // fd, poller监听的对象
  int events_;      // 注册sockfd感兴趣的event事件
  int revents_;      // poller返回具体发生的事件（epoll特色）
  int index_;

  std::weak_ptr<void> tie_; // void接收所有类型：绑定自己(shared _from_this？：返回一个当前类的std::share_ptr,)
  bool tied_;

  // 因为Channel通道里能够获取fd最终发生的具体事件revents，所以他负责调用具体事件的回调操作
  ReadEventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;

};