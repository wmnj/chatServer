#include "chatservice.hpp"
#include <muduo/base/Logging.h>
#include <functional>
#include <vector>
#include "public.hpp"

using namespace std;
using namespace muduo;
using namespace placeholders;

ChatService::ChatService(/* args */)
{
    ChatService::_msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    // ChatService::_msgHandlerMap.insert({LOGIN_MSG_ACK, std::bind(&ChatService::login, this, _1, _2, _3)});
    ChatService::_msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    // ChatService::_msgHandlerMap.insert({REG_MSG_ACK, std::bind(&ChatService::login, this, _1, _2, _3)});
    ChatService::_msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    ChatService::_msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    // CREATE_GROUP_MSG = 7, // 创建群组 ADD_GROUP_MAG = 8, // 加群 GROUP_CHAT_MSG = 9, // 群聊天
    ChatService::_msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    ChatService::_msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    ChatService::_msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
    ChatService::_msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
    if (_redis.connect()) {
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage,this,_1,_2));
    }
}

ChatService *ChatService::instance()
{
    static ChatService sercice;
    return &sercice;
}

MsgHandler ChatService::getHander(int msgid)
{
    auto it = ChatService::_msgHandlerMap.find(msgid);
    // 不要抛出异常 当msgid没有对应的handler的时候不得不处理该异常 ，返回空对象并打印日志 上层可以灵活处理
    if (it == ChatService::_msgHandlerMap.end())
    {
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp)
        {
            LOG_ERROR << "msgid: " << msgid << "can not find handler!!!";
        };
    }
    return (*it).second;
}

void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 从tcp连接map中删除连接 设置状态为offline
    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(ChatService::_connMutex);
        auto it = ChatService::_userConnMap.find(userid);
        if (it != ChatService::_userConnMap.end())
        {
            ChatService::_userConnMap.erase(it);
        }
    }

    // 用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(userid);

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    ChatService::_userModel.updateState(user);
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    // 客户端断开连接 删除userconmap中的数据
    User user;
    {
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                // 从map表删除用户的链接信息
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }
    // 用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(user.getId());

    // 将状态设置为offline
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

void ChatService::reset()
{
    // 将用户的状态设置为offline
    _userModel.resetState();
}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    // LOG_INFO << "log success!";
    // 1. 获取用户信息 id pwd;
    int id = js["id"].get<int>();
    string pwd = js["password"];
    LOG_INFO << id << " " << pwd;
    // 2.用usermodel查询数据库
    User user = _userModel.query(id);
    json response;
    response["msgid"] = LOGIN_MSG_ACK;
    if (user.getId() != -1 && user.getPwd() == pwd)
    {
        // 3. 登陆成功 验证是否重复登陆
        if (user.getState() == "online")
        {
            response["errno"] = 2;
            response["errmsg"] = "This count has been use, do not login again!";
            conn->send(response.dump());
        }
        else
        {
            // 登陆成功 记录用户连接信息
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }
            // 登陆成功 向redis订阅channel（id）
            _redis.subscribe(id);

            // 设置数据库状态为online
            user.setState("online");
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            _userModel.updateState(user);
            // 从数据库获取用户的离线信息
            std::vector<string> vec = _offlineModel.querymsg(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 读取消息后删除用户的离线消息
                _offlineModel.removemsg(id);
            }
            // 返回用户的好友信息
            vector<User> vec1 = _friendModel.query(id);
            if (!vec1.empty())
            {
                vector<string> vec2;
                for (User u : vec1)
                {
                    json fs;
                    fs["id"] = u.getId();
                    fs["name"] = u.getName();
                    fs["state"] = u.getState();
                    vec2.push_back(fs.dump());
                }
                response["friends"] = vec2;
            }
            // 查询用户的群组信息
            vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if (!groupuserVec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for (Group &group : groupuserVec)
                {
                    json grpjson;
                    grpjson["id"] = group.getGroupId();
                    grpjson["groupname"] = group.getGroupName();
                    grpjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }

                response["groups"] = groupV;
            }
            conn->send(response.dump());
        }
    }
    else
    {
        // 登陆失败
        response["errno"] = 1;
        response["errmsg"] = "login failure!";
        conn->send(response.dump());
    }
}

void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    // LOG_INFO << "reg success!";
    // 接收数据 创建user对象 执行插入操作 返回结果
    string name = js["name"];
    string pwd = js["password"];
    User user;
    user.setName(name);
    user.setPwd(pwd);
    if (_userModel.insert(user))
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["id"] = user.getId();
        response["errno"] = 0;
        conn->send(response.dump());
    }
    else
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    // {"msgid":5,"id":1,"name":"qw","toid":2,"msg":"hello."}
    // 收到消息后 获取接收方的id 并将消息发送给接收方
    int toid = js["toid"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            it->second->send(js.dump());
            return;
        }
    }
    // 查询toid是否在线
    User user = _userModel.query(toid);
    if (user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
        return;
    }

    // 对方已经离线 在数据库中存储离线消息
    _offlineModel.insertmsg(toid, js.dump());
}

// 添加好友业务
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 获取用户id 好友id 插入到表Frinend中
    int id = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    _friendModel.insert(id, friendid);
}

// 创建群组
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 获取 群名 群描述信息
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];
    Group grp(-1, name, desc);
    // 将信息插入AllGroup中 已经将组id记录在grp中
    // 将群主和组的关系 加入到GroupUser中
    if (_groupModel.insert(grp))
    {
        _groupModel.addGroup(userid, grp.getGroupId(), "creator");
    }
}
// 加群
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}
// 群聊天
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 发消息的人 所在的群id 发的消息转发给群内的所有人
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    std::vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);
    lock_guard<mutex> lock(_connMutex);
    for (int uid : useridVec)
    {
        // 根据id获取tcp连接
        auto it = _userConnMap.find(uid);
        if (it != _userConnMap.end())
        {
            // 用户在线 直接转发
            it->second->send(js.dump());
        }
        else
        {
            // 查询toid是否在线
            User user = _userModel.query(uid);
            if (user.getState() == "online")
            {
                _redis.publish(uid, js.dump());
            }
            else
            {
                // 存储离线群消息
                _offlineModel.insertmsg(uid, js.dump());
            }
        }
    }
}

// 将从redis中订阅的消息发送给指定的客户端
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }
    _offlineModel.insertmsg(userid, msg);
}