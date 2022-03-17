#include"Poller.h"

#include"Channel.h"

Poller::Poller(EventLoop *loop) 
    : ownerLoop_(loop) {
    }

bool Poller::hasChannel(Channel *channel) const {
  auto it = channels_.find(channel->fd());
  return it != channels_.end() && it->second == channel;
}

// 面试要点！不可实现 newDefaultPoller ：继承体系出错， 应单独在一个源文件中实现