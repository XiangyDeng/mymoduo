#include <mymuduo/TcpServer.h>
#include <mymuduo/Logger.h>
#include <mymuduo/TcpConnection.h>

#include <string>

class EchoServer {
 public:
  EchoServer(EventLoop *loop,
            const InetAddress &addr,
            const std::string &name) 
      : server_(loop_, addr, name)
      , loop_(loop) {
    // 注册回调函数
    server_.setConnectionCallback(
        std::bind(&EchoServer::OnConnection, this, std::placeholders::_1)
    );
    server_.setMessageCallback(
        std::bind(&EchoServer::OnMessage, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    );

    // 设置合适的loop线程数量(subReaction)，加上主线程共4个线程
    server_.setThreadNum(3);
}

  void start() {
    server_.start();
  }

 private:
  // 设置连接建立或断开的回调函数
  void OnConnection(const TcpConnectionPtr &conn) {
      if (conn->connected()) {
        LOG_INFO("Conn UP : %s", conn->peerAddress().toIpPort().c_str());
      } else {
        LOG_INFO("Conn DOWN : %s", conn->peerAddress().toIpPort().c_str());
      }
  }

  // 设置可读写事件的回调函数
  void OnMessage(const TcpConnectionPtr &conn, 
                Buffer *buf,
                Timestamp time) {
      std::string msg = buf->retrieveAllAsString();
      msg = "\n";
      conn->send(msg);
      msg = "Helle, User. \n";
      conn->send(msg);
      msg = "I can hear you! \n";
      conn->send(msg);
      LOG_INFO("Echo back: ", msg);
      conn->shutdown(); // 回调 EPOLLHUP => closeCallback_
  }

  EventLoop *loop_;
  TcpServer server_;
};

int main() {
  EventLoop loop; // 创建mainLoop， 就在本主线程中。
  InetAddress addr(8000);
  // 之后在TcpServer类的EventLoopThreadPool类中建立subLoop（同时创建新线程:EventLoopThread::threadFunc中的Thread::start()，以智能指针的形式创建新线程）
  EchoServer server(&loop, addr, "EchoServer-01");  // Accepter non-blocking listenfd create bind
  server.start(); // listen loopthread listenfd => acceptChannel => mainLoop => 
  loop.loop();   // 启动mainLoop的底层Poller
  return 0;
}