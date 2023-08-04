#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

using namespace muduo;
using namespace muduo::net;

class chatServer
{
private:
    TcpServer _server; 
    EventLoop *_loop;
    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);

public:
    chatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg);
    void start(); 
};

#endif
