#include "EpollPoller.h"

#include <strings.h>
#include <errno.h>
#include <unistd.h>
#include "Logger.h"
#include "Channel.h"

// channel 未添加到poller中
const int kNew = -1;    
// channel 已添加到poller中
const int kAdded = 1; // channel的成员index_ = -1  
// channel 从poller中删除
const int kDeleted = 2; 

EpollPoller::EpollPoller(EventLoop *loop) 
    : Poller(loop),
      epollfd_(::epoll_create(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
      if(epollfd_ < 0) {
        LOG_FATAL("epoll_create error:%d \n ", errno);
      }
    
    }

EpollPoller::~EpollPoller() {
  ::close(epollfd_);
}

// epoll_wait
Timestamp EpollPoller::poll(int timeoutMS, ChannelList *activeChannel) {
  // DEBUG 更合理
  LOG_INFO("func=%s => fd total count:%lu\n", __FUNCTION__, channels_.size());

  int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMS);
  int saveErrno = errno;
  Timestamp now(Timestamp::now());

  if (numEvents > 0) {
    LOG_INFO("%d events happened \n", numEvents);
    fillActiveChannels(numEvents, activeChannel);

    if (numEvents == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  } else if (numEvents == 0) {
    LOG_DEBUG("%s timeout! \n", __FUNCTION__);
  } else {
    if (saveErrno != EINTR) {
      errno = saveErrno;
      LOG_ERROR("EPollPoller::poll() err!");
    }
  }
  return now;
}

// Channel update remove => EventLoop updateChannel remove Channel => Poller updateChannel remove Channel => Poller
/**
 * @brief             EventLoop => Poller.poll(epoll_wait)
 *            ChannelList     Poller
 *                            ChannelMap <fd, channel*>
 *                            Events <= epoll_event (vector)
 * 
 * @param channel 输入待更新的Channel
 */
void EpollPoller::updateChannel(Channel *channel) {
    const int index = channel->index();
    LOG_INFO("func=%s => fd=%d, events=%d, index=%d \n", __FUNCTION__, channel->fd(), channel->events(), channel->index());
    if (index == kNew || index == kDeleted) {
      if (index == kNew) { // channel 从来未添加到poller中
        int fd = channel->fd();
        channels_[fd] = channel;
      }

      channel->set_index(kAdded);
      update(EPOLL_CTL_ADD, channel);
    } else {
      int fd = channel->fd();
      if (channel->isNoneEvent()) {
        update(EPOLL_CTL_DEL, channel);
        channel->set_index(kDeleted);
      } else {
        update(EPOLL_CTL_MOD, channel);
      }
    }
}

// 从Poller中删除channel
void EpollPoller::removeChannel(Channel *channel) {
  int fd = channel->fd();
  channels_.erase(fd);

  LOG_INFO("func=%s => fd=%d \n", __FUNCTION__, channel->fd());

  int index = channel->index();
  if (index == kAdded) {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->set_index(kNew);
}

// 填写活跃的链接(revents对应的channel)
void EpollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const {
  for (int i=0; i < numEvents; ++i) {
    Channel *channel = static_cast<Channel*>(events_[i].data.ptr);
    channel->set_revents(events_[i].events);
    activeChannels->push_back(channel); // Eventloop拿到了poller给它返回的所有发生事件的channel列表
  }
}

// 更新Channel通道（epoll_ctl的调用: add/mod/delete）
void EpollPoller::update(int operation, Channel *channel) {
  epoll_event event;
  bzero(&event, sizeof event);
  int fd = channel->fd();

  event.events = channel->events();
  event.data.ptr = channel;
  event.data.fd = fd;

  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL) {
      LOG_ERROR("epoll ctl del error:%d\n", errno);
    } else {
      LOG_ERROR("epoll ctl add/mod error:%d\n", errno);
    }
  }
} 