#ifndef ChatService_H
#define ChatService_H

#include <functional>
#include <unordered_map>
#include <muduo/net/TcpConnection.h>
#include <mutex>
#include "json.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "group.hpp"
#include "groupuser.hpp"
#include "redis.hpp"

using namespace muduo;
using namespace muduo::net;
using namespace std;
using json = nlohmann::json;

using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;
class ChatService
{
private:
    ChatService();
    // 存储消息id和其对应的业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;
    // 存储在线用户的通信连接
    // 记录用户id和连接的映射 如果A->B通信，服务器收到消息 需要知道发给哪个连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;
    // 定义互斥锁，保证_userConnMap的线程安全
    mutex _connMutex;
    // 数据操作类对象
    UserModel _userModel;
    OfflineMsgModel _offlineModel;
    FriendModel _friendModel;
    GroupModel _groupModel;
    //  redis操作对象
    Redis _redis;

public:
    static ChatService *instance();
    void login(const TcpConnectionPtr &conn, json &js, Timestamp);
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp);
    // 发消息
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp);
    // 添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 创建群组
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 加群
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 群聊天
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理注销业务
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void clientCloseException(const TcpConnectionPtr &conn);
    void reset();
    MsgHandler getHander(int msgid);
    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int,std::string);
};

#endif