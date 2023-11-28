#pragma once

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"
#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>

class Channel;
class Poller;

class EventLoop: noncopyable {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_; }
    void runInLoop(Functor cb);  //在当前loop执行cb
    void queueInLoop(Functor cb);  //把cb放入队列，唤醒loop所在线程，执行cb

    void wakeup();  //唤醒loop所在线程
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    bool isInLoopThread() const {
        threadId_ == CurrentThread::tid();
    }

private:
    void handleRead();
    void doPendingFunctors();

    using ChannelList = std::vector<Channel*>;

    std::atomic_bool looping_;  //原子操作，通过cas实现
    std::atomic_bool quit_;  //标识退出loop
    const pid_t threadId_;  //当前loop所在线程id

    Timestamp pollReturnTime_;  //poller返回发生事件的channels的时间
    std::unique_ptr<Poller> poller_;

    int wakeupFd_;  //当mainloop获取一个新用户的channel，通过轮询算法选择一个subloop，通过该成员唤醒subloop处理channel
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;
    Channel* currentActiveChannels_;

    std::atomic_bool callingPendingFunctors_;  //标识当前loop是否有需要执行的cb
    std::vector<Functor> pendingFunctors_;
    std::mutex mutex_; //互斥锁，保护pendingFunctors_线程安全操作

};