#pragma once

#include "noncopyable.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include "Callbacks.h"
#include "EventLoopThreadPool.h"
#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>

class TcpServer: noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    enum Option {
        kNoReusePort,
        kReusePort
    };

    TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg, Option option = kNoReusePort);
    ~TcpServer();

    //设置底层subloop个数
    void setThreadNum(int numThreads);
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }

private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnecitonInLoop(const TcpConnectionPtr& conn);

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
    EventLoop* loop_;  //baseloop
    const std::string ipPort_;
    const std::string name_;
    //运行在mainloop，监听新连接事件
    std::unique_ptr<Acceptor> acceptor_;
    //one loop per thread
    std::shared_ptr<EventLoopThreadPool> threadPool_;

    //有新连接时的cb
    ConnectionCallback connectionCallback_;
    //有读写消息时的cb
    MessageCallback messageCallback_;
    //消息发送完成后的cb
    WriteCompleteCallback writeCompleteCallback_;

    ThreadInitCallback threadInitCallback_;
    std::atomic_int started_;

    int nextConnId_;
    ConnectionMap connections_;  //所有连接
};