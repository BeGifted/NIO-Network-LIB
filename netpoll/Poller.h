#pragma once

#include "noncopyable.h"
#include <vector>
#include <unordered_map>

class Channel;
class EventLoop;
class Timestamp;

class Poller: noncopyable {
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);
    virtual ~Poller();

    //给所有IO复用保留统一的接口
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;

    //判断channel是否在当前poller中
    bool hasChannel(Channel* channel) const;

    //eventloop可以通过该接口获取默认的IO复用的具体实现
    static Poller* newDefaultPoller(EventLoop* loop);

protected:
    //key表示sockfd，value表示sockfd所属的channel通道类型
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop* ownerLoop_;  //poller所属事件循环
};