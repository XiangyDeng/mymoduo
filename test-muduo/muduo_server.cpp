// TcpServer TcpClient
// epoll + 线程池
// 区分开网络IO代码与业务代码（用户的连接与断开，用户的可读写事件）
#include <net/TcpServer.h>
#include <net/EventLoop.h>
#include <iostream>
#include <string>
#include <functional>   //bind()绑定器

using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;

// 基于moduo网络库开发服务器程序
/*
1. 组合TcpServer对象
2. 创建EventLoop事件循环对象的指针
3. 明确TcpServer构造函数需要什么函数，输出ChatServer的构造函数
4. 在当前服务器类的构造函数中注册用户连接创建与断开的回调函数以及用户读写事件的回调函数
5. 设置合适的服务端线程数量，muduo库会自己分配IO线程与工作线程
++ muduo网络库使得用户可以只关注回调函数的书写！！！
*/
class ChatServer
{
public:
    ChatServer(EventLoop *loop,                  // 事件循环
                const InetAddress &listenAddr,  // IP + Port
                const string &nameArg           // 服务器的名字
                ):_server(loop, listenAddr, nameArg), _loop(loop)
    {
        // 给服务器注册用户连接的创建和断开 回调（绑定器实现）
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
        // 给服务器注册用户读写事件 回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));
        // 设置服务器端的线程数量: 1个IO线程 + 3个工作线程
        _server.setThreadNum(4);     
    }
    ~ChatServer()
    {
        cout << "Connection closed!" << endl;
        _loop->quit();
    }

    // 开启事件循环
    void start() 
    {
        _server.start();
    }

private:
    // 回调函数：专门处理用户的连接创建和断开 epoll listenfd accept
    void onConnection(const TcpConnectionPtr &conn)
    {
        if(conn->connected())
        {
            cout << conn->peerAddress().toIpPort()  << " -> " 
                << conn->localAddress().toIpPort() << " state:online " << endl;
        }
        else{
            cout << conn->peerAddress().toIpPort()  << " -> " 
                << conn->localAddress().toIpPort() << " state:offline " << endl;    
            // conn->shutdown();   // close(fd)
            // _loop->quit();
        }

    }

    // 回调函数：专门处理用户的读写事件
    void onMessage(const TcpConnectionPtr &conn,    // 连接
                    Buffer *buffer,                    // 读写缓冲区
                    Timestamp time                  // 接收到数据的时间戳（时间信息）
                    )
    {
        // 讲缓冲区内的所有数据转为string
        string buf = buffer->retrieveAllAsString();
        cout << "recv data: " << buf << " time: " << time.toString() << endl;
        conn->send(buf);
    }

    muduo::net::TcpServer _server;
    EventLoop *_loop;               // epoll
};

int main()
{
    EventLoop loop; // 类似epoll
    InetAddress addr("127.0.0.1", 6510);
    ChatServer server(&loop, addr, "ChatServer"); 

    server.start(); // 启动服务：listenfd epoll_ctl => epoll
    loop.loop();    // epoll_wait() 以阻塞的方式等待新用户连接， 已连接用户的读写事件等方法

    return 0;

}