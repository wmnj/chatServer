#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include <vector>
#include <string>
using namespace std;

class OfflineMsgModel
{
public:
    // 存储用户的离线消息
    void insertmsg(int id, string msg);
    // 删除用户的离线消息
    void removemsg(int id);
    // 查询用户的离线消息
    std::vector<string> querymsg(int id);
};

#endif