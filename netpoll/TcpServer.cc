#include "TcpServer.h"
#include "Logger.h"
#include "EventLoop.h"

EventLoop* CheckLoopNotNull(EventLoop* loop) {
    if (loop == nullptr) {
        LOG_FATAL("%s:%s:%d mainloop is null", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg, Option option)
    :loop_(CheckLoopNotNull(loop))
    ,ipPort_(listenAddr.toIpPort())
    ,name_(nameArg)
    ,acceptor_(new Acceptor(loop, listenAddr, option == kReusePort))
    ,threadPool_(new EventLoopThreadPool(loop, name_))
    ,connectionCallback_()
    ,messageCallback_()
    ,nextConnId_(1) {
    //有新用户连接时，执行该cb
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {

}

void TcpServer::setThreadNum(int numThreads) {
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
    if (started_++ == 0) {  //防止一个tcpserver对象被start多次
        threadPool_->start(threadInitCallback_);  //启动底层loop线程池
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {

}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {

}

void TcpServer::removeConnecitonInLoop(const TcpConnectionPtr& conn) {

}
