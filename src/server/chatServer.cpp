#include "chatServer.hpp"
#include "json.hpp"
#include "chatservice.hpp"

#include <iostream>
#include <functional>
#include <string>

using namespace std;
using namespace placeholders;
using json = nlohmann::json;

chatServer::chatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop)
{
    _server.setConnectionCallback(std::bind(&chatServer::onConnection, this, _1));
    _server.setMessageCallback(std::bind(&chatServer::onMessage, this, _1, _2, _3));
    _server.setThreadNum(26);
}
void chatServer::start(){
    _server.start();
}
// 上报连接相关信息的回调函数
void chatServer::onConnection(const TcpConnectionPtr& conn)
{
    // 客户端断开连接
    if (!conn->connected()) {
        // 待优化 存储连接和id的映射
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}
// 上报消息接受相关的回调函数
void chatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    // 接受客户端的消息
    string msg = buf->retrieveAllAsString();
    // 数据的反序列化
    json js = json::parse(msg);
    auto msghandler = ChatService::instance()->getHander(js["msgid"].get<int>());
    // 实现网络层和业务层的代码解绑：通过事件绑定的方式，根据不同的消息类型获取相应的事件处理handler
    msghandler(conn,js,time);
    // 根据不同的消息类型 执行不同的回调函数 
    
}