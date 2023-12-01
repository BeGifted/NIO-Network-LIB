#include "TcpServer.h"
#include "Logger.h"
#include "Callbacks.h"
#include <strings.h>


TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg, Option option)
    :loop_(CHECK_NOTNULL(loop))
    ,ipPort_(listenAddr.toIpPort())
    ,name_(nameArg)
    ,acceptor_(new Acceptor(loop, listenAddr, option == kReusePort))
    ,threadPool_(new EventLoopThreadPool(loop, name_))
    ,connectionCallback_(defaultConnectionCallback)
    ,messageCallback_(defaultMessageCallback)
    ,started_(0)
    ,nextConnId_(1) {
    //有新用户连接时，执行该cb
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    LOG_INFO("TcpServer::~TcpServer [%s] destructing", name_.c_str());
    for (auto& item : connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
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

//有一个新的客户端的连接，acceptor会执行这个cb
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    EventLoop* ioLoop = threadPool_->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s", name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());
    //通过sockfd获取其绑定的本地ip地址和端口
    sockaddr_in local;
    bzero(&local, sizeof local);
    socklen_t addrlen = static_cast<socklen_t>(sizeof local);
    if (::getsockname(sockfd, (sockaddr*)&local, &addrlen) < 0) {
        LOG_ERROR("getlocaladdr");
    }
    InetAddress localAddr(local);

    //根据连接成功的sockfd，创建TcpConnection连接对象
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    //用户设置给tcpserver->tcpconnection->channel->poller->notify channel cb
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnecitonInLoop, this, conn));
}

void TcpServer::removeConnecitonInLoop(const TcpConnectionPtr& conn) {
    LOG_INFO("TcpServer::removeConnecitonInLoop [%s] - connection %s", name_.c_str(), conn->name().c_str());

    connections_.erase(conn->name());
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));

}
