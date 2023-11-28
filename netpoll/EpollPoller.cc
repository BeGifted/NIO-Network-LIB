#include "EpollPoller.h"
#include "Channel.h"
#include "Logger.h"
#include <errno.h>
#include <string.h>
#include <strings.h>
 #include <unistd.h>


//channel未添加到poller
const int kNew = -1;  //channel的成员index_ = -1
//channel已添加到poller
const int kAdded = 1;
//channel从poller中删除
const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop* loop)
    :Poller(loop)
    ,epollfd_(::epoll_create1(EPOLL_CLOEXEC))  //在一个进程中使用 epoll_create1(EPOLL_CLOEXEC) 创建了一个 epoll 实例，如果之后调用 fork() 创建了子进程并执行了一个新的程序，这个 epoll 实例会在新程序执行时自动关闭，确保在新程序中不会不必要地保留文件描述符。
    ,events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        LOG_FATAL("epoll_create error: %d", errno);
    }
}


EpollPoller::~EpollPoller() {
    ::close(epollfd_);
}

Timestamp EpollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    LOG_INFO("func=%s => fd total count: %d", __FUNCTION__, channels_.size());
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());

    if (numEvents > 0) {
        LOG_INFO("%d events happened", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if (numEvents == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (numEvents == 0) {
        LOG_DEBUG("%s timeout", __FUNCTION__);
    } else {
        if (saveErrno != EINTR) {
            errno = saveErrno;
            LOG_ERROR("EpollPoller::poll() err");
        }
    }
    return now;
}

//channel update -> eventloop update -> poller update
void EpollPoller::updateChannel(Channel* channel) {
    const int index = channel->index();
    LOG_INFO("func=%s fd=%d, events=%d, index=%d", __FUNCTION__, channel->fd(), channel->events(), index);

    if (index == kNew || index == kDeleted) {
        if (index == kNew) {
            int fd = channel->fd();
            channels_[fd] = channel;
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {  //channel已经在poller上注册过
        int fd = channel->fd();
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(Channel* channel) {
    LOG_INFO("func=%s fd=%d", __FUNCTION__, channel->fd());

    int fd = channel->fd();
    channels_.erase(fd);

    int index = channel->index();
    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void EpollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
    for (int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}


void EpollPoller::update(int operation, Channel* channel) {
    epoll_event event;
    bzero(&event, sizeof event);
    int fd = channel->fd();

    event.events = channel->events();
    event.data.fd = fd;
    event.data.ptr = channel;
    

    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_ERROR("epoll_ctl del error: %d", errno);
        } else {
            LOG_FATAL("epoll_ctl add/mod error: %d", errno);
        }
    }
}
