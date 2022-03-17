#pragma once

#include <vector>
#include <sys/epoll.h>
#include ""

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

  void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
  void update(int operation, Channel *channel);

  using EventList = std::vector<epoll_event>;

  int epollfd_;
  EventList events_;

};