#include "EpollPoller.h"
#include "Logger.h"
#include "Channel.h"

#include <errno.h>
#include <unistd.h>
#include <strings.h>

// channel未添加到poller中
const int kNew = -1;  // channel的成员index_ = -1
// channel已添加到poller中
const int kAdded = 1;
// channel从poller中删除
const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop *loop)
    : Poller(loop)
    , epollfd_(::epoll_create1(EPOLL_CLOEXEC))  // fork不遗传
    , events_(kInitEventListSize)  // vector<epoll_event>
{
    if (epollfd_ < 0)
    {
        LOG_FATAL("epoll_create error:%d \n", errno);
    }
}

EpollPoller::~EpollPoller() 
{
    ::close(epollfd_);
}

Timestamp EpollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    // 实际上应该用LOG_DEBUG输出日志更为合理
    LOG_INFO("func=%s => fd total count:%lu \n", __FUNCTION__, channels_.size());

    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());

    if (numEvents > 0)
    {
        LOG_INFO("%d events happened \n", numEvents);
        // 将epoll_wait获取的事件与每个channel相关联
        fillActiveChannels(numEvents, activeChannels);
        // TODO:此处为什么不是 >= 
        if (numEvents == events_.size())
        {   
            // 二倍扩size
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        LOG_DEBUG("%s timeout! \n", __FUNCTION__);
    }
    else
    {
        if (saveErrno != EINTR)
        {
            errno = saveErrno;
            LOG_ERROR("EpollPoller::poll() err!");
        }
    }
    return now;
}

// channel update remove => EventLoop updateChannel removeChannel => Poller updateChannel removeChannel
/**
 *            EventLoop  =>   poller.poll
 *     ChannelList      Poller
 *                     ChannelMap  <fd, channel*>   epollfd
 */ 
void EpollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();
    LOG_INFO(" EpollPoller::updateChannel func=%s => fd=%d events=%d index=%d \n", __FUNCTION__, channel->fd(), channel->events(), index);

    if (index == kNew || index == kDeleted)
    {
        if (index == kNew)
        {
            int fd = channel->fd();
            channels_[fd] = channel;    // 1. 添加poller中的channel
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel); // 2. 使用epoll_ctl 添加到内核事件表
    }
    else  // channel已经在poller上注册过了
    {
        int fd = channel->fd();
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

// 从poller中删除channel, 根据channel中fd
// 1. 删除channel：删除Poller类的channels_（hashmap）中的channel
// 2. 删除内核事件表中的event：调用epoll_ctl删除该loop eventfd_维护的内核事件表中的事件，其中有ptr指向channel
void EpollPoller::removeChannel(Channel *channel) 
{
    int fd = channel->fd();
    channels_.erase(fd);

    LOG_INFO("EpollPoller::removeChannel func=%s => fd=%d\n", __FUNCTION__, fd);
    
    int index = channel->index();
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

// 填写活跃的连接
void EpollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for (int i=0; i < numEvents; ++i)
    {   
        // 让每个event事件分别指向不同的channel 
        Channel *channel = static_cast<Channel*>(events_[i].data.ptr);
        // 将events填入对应的channel, 再通过引用返回给EventLoop
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel); // EventLoop就拿到了它的poller给它返回的所有发生事件的channel列表了
    }
}

// 更新channel通道 epoll_ctl add/mod/del
void EpollPoller::update(int operation, Channel *channel)
{
    // 需要重新定义epoll_event ctl
    epoll_event event;
    bzero(&event, sizeof event);
    
    int fd = channel->fd();

    event.events = channel->events();
    event.data.fd = fd;     // sockfd
    event.data.ptr = channel;
    //once epollfd_ per loop
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl add/mod error:%d\n", errno);
        }
    }
}