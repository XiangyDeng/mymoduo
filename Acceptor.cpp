#include "Acceptor.h"
#include "Logger.h"
#include "InetAddress.h"

#include <sys/types.h>    
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

static int createNonblocking() {
  int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (sockfd < 0) {
    LOG_FATAL("%s:%s:%d listen socket createNonblocking error:%d \n ", __FILE__, __FUNCTION__, __LINE__, errno);
  }
  return sockfd;
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport) 
    : loop_(loop)
    , acceptSocket_(createNonblocking())  // create socket 接口
    , acceptChannel_(loop, acceptSocket_.fd())
    , listenning_(false) {
  
  acceptSocket_.setReuseAddr(true);
  acceptSocket_.setReusePort(true);
  acceptSocket_.bindAddress(listenAddr);  // bind
  // TcpServer::start() => Acceptor.listen => 有新用户连接，要执行一个回调（connfd => channel => subloop(subReactor)）
  // baseloop => acceptChannel_(listenfd) =>
  acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead,this));
} 

Acceptor::~Acceptor() {
  acceptChannel_.disableAll();
  acceptChannel_.remove();
}

void Acceptor::listen() {
  listenning_ = true;
  acceptSocket_.listen(); // listen
  acceptChannel_.enableReading();
}

// listenfd有事件发生了，就是有新用户连接了
// 服务器（socket => bind => listen）-> 客户端（socket => connect）-> 服务器（accept => handleRead）
void Acceptor::handleRead() {
  InetAddress peeraddr;
  int connfd = acceptSocket_.accept(&peeraddr);
  if (connfd >= 0) {
    if (newConnectionCallback_) {
      newConnectionCallback_(connfd, peeraddr); // 轮询找到subloop，唤醒，分发当前的新客户端的channel
    } else {
      ::close(connfd);
    }
  } else {
    LOG_ERROR("%s:%s:%d accept error:%d \n ", __FILE__, __FUNCTION__, __LINE__, errno);
    if (errno == EMFILE) {
      LOG_ERROR("%s:%s:%d sockfd reacher limit. \n ", __FILE__, __FUNCTION__, __LINE__);
    }
  }
}