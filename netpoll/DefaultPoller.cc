#include "Poller.h"
#include <stdlib.h>


Poller* Poller::newDefaultPoller(EventLoop* loop) {
    if (::getenv("NETPOLL_USE_POLL")) {
        return nullptr;  //生成poll实例
    } else {
        return nullptr;  //生成epoll实例
    }
}