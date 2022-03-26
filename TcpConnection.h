#pragma once

#include <memory>
#include <string>
#include <atomic>

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Timestamp.h"
#include "Buffer.h"

class Channel;
class EventLoop;
class Socket;
class Callbacks;

/**
 * @TcpServer => Acceptor => 有一个新用户连接，通过accept函数拿到connfd
 * 
 * =>TcpConnection 设置回调 => Channel => Poller => Channel的回调操作
 */
class TcpConnection : noncopyable , public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop *loop,
                const std::string &nameArg,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
  ~TcpConnection();

  EventLoop* getLoop() const { return loop_; }
  const std::string name() const { return name_; }
  const InetAddress& localAddress() const { return localAddr_; }
  const InetAddress& peerAddress() const { return peerAddr_; }

  bool connected() const { return state_ == kConnected; }

  // 发送数据
  void send(const std::string& buf);
  // 关闭连接
  void shutdown();

  void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
  void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
  void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
  void setHighWaterMarkCallback(const HighWaterMarkCallback& cb) { highWaterMarkCallback_ = cb; }
  void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

  // 连接建立
  void connectEstablished();
  // 连接销毁
  void connectDestroyed();

 private:
  enum stateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
  void setState(stateE state) { state_ = state; }

  void handleRead(Timestamp receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();

  void sendInLoop(const void* message, size_t len);
  void shutdownInLoop();

  EventLoop *loop_; // 这里不是baseloop，因为TcpConnection都是在subloop里进行管理
  const std::string name_;

  std::atomic_int state_;
  bool reading_;

  // 这里与Acceptor类似 Accept在mainloop里，TcpConnection是在subLoop里
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;

  const InetAddress localAddr_;
  const InetAddress peerAddr_;

  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  HighWaterMarkCallback highWaterMarkCallback_;
  CloseCallback closeCallback_;
  size_t highWaterMark_;

  Buffer inputBuffer_;
  Buffer outputBuffer_;
};