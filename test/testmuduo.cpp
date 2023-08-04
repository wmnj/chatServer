#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <boost/bind.hpp>
#include <muduo/net/EventLoop.h>
#include <iostream>

/*
muduo库提供了tcpserver和tcpclient类 分别用于服务端和客户端的编程
编程步骤：
1. 组装tcpserver类
2. 创建eventloop循环对象
3. 明确tcpserver构造函数需要的参数
4. 当前服务器类中构造回调函数
*/
class MyChatServer
{
private:
    muduo::net::TcpServer server_;
    void onConnection(const muduo::net::TcpConnectionPtr &conn);
    void onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf, muduo::Timestamp time);

public:
    MyChatServer(muduo::net::EventLoop *loop,
                 const muduo::net::InetAddress &listenAddr) : server_(loop, listenAddr, "mychatserver")
    {
        // 注册用户连接和断开的回调函数
        server_.setConnectionCallback(boost::bind(&MyChatServer::onConnection, this, _1));
        // 给服务器注册用户读写的回调函数
        server_.setMessageCallback(boost::bind(&MyChatServer::onMessage, this, _1, _2, _3));
        // 设置服务端的线程数量 1个IO主线程 n-1个业务处理函数
        server_.setThreadNum(4);
    }
    void start() {
        server_.start();
    }
};

// 处理客户端的连接和断开
void MyChatServer::onConnection(const muduo::net::TcpConnectionPtr &conn) {
    //    输出客户端的信息
    std::cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << 
    " is " << (conn->connected() ? "UP" : "DOWN") << std::endl;
    
}
void MyChatServer::onMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp time) {
    // 接受客户端的消息并回显
    muduo::string msg(buf->retrieveAllAsString());
    std::string a = "a";
    LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, data receivetd at" << time.toString();
    conn->send("hello, sucess connect!"); 
}

int main()
{
    LOG_INFO << "pid = " << getpid();
    muduo::net::EventLoop loop;
    muduo::net::InetAddress listenAddr(8888);
    MyChatServer server(&loop, listenAddr);
    server.start();
    loop.loop();
}