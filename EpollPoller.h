#pragma once

#include <vector>
#include <sys/epoll.h>
#include "Poller.h"
#include "EventLoop.h"
#include "Timestamp.h" 

// 只使用了指针类型，故只使用前置声明
class Channel;

/**
 * @brief epoll的使用的
 *        epoll_create
 *        epoll_ctl   add/mod/del
 *        epoll_wait
 * 
 */
class EpollPoller : public Poller {
 public:
  EpollPoller(EventLoop *loop);
  ~EpollPoller() override;   // 可执行编译器检查虚函数

  // 重写抽象基类IO复用的接口
  Timestamp poll(int timeoutMS, ChannelList *activeChannel) override;
  void updateChannel(Channel *channel) override;
  void removeChannel(Channel *channel) override;



 private:
  static const int kInitEventListSize = 16;

  // 填写活跃的链接
  void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
  // 更新Channel通道（epoll_ctl的调用）
  void update(int operation, Channel *channel);

  using EventList = std::vector<epoll_event>;

  int epollfd_;
  EventList events_;

};