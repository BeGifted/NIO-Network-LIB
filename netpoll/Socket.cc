#include "Socket.h"
#include "Logger.h"
#include "InetAddress.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <strings.h>
#include <unistd.h>

Socket::~Socket() {
    close(sockfd_);
}

void Socket::bindAddresss(const InetAddress& localaddr){
    int rt = ::bind(sockfd_, (sockaddr*)localaddr.getSockAddr(), sizeof(sockaddr_in));
    if (rt != 0) {
        LOG_FATAL("bind sockfd: %d fail", sockfd_);
    }
}

void Socket::listen(){
    int rt = ::listen(sockfd_, 1024);
    if (rt != 0) {
        LOG_FATAL("listen sockfd:%d fail", sockfd_);
    }
}

int Socket::accept(InetAddress* peeraddr){
    sockaddr_in addr;
    socklen_t len = sizeof addr;
    bzero(&addr, sizeof addr);
    int connfd = ::accept4(sockfd_, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd >= 0) {
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}

void Socket::shutdownWrite(){
    if (::shutdown(sockfd_, SHUT_WR) < 0) {
        LOG_ERROR("shutdownWrite error");
    }
}

void Socket::setTcpNoDelay(bool on){
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReuseAddr(bool on){
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReusePort(bool on){
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setKeepAlive(bool on){
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
}
