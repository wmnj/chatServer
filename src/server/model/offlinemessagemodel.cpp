#include "db.hpp"
#include "offlinemessagemodel.hpp"

// 将用户id的离线消息存储到数据库表中
void OfflineMsgModel::insertmsg(int id, string msg)
{
    // 获取数据库连接
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage values(%d,'%s')", id, msg.c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

void OfflineMsgModel::removemsg(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid = %d", id);
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

std::vector<string> OfflineMsgModel::querymsg(int id)
{
    char sql[1024];
    sprintf(sql, "select Message from OfflineMessage where userid=%d", id);
    MySQL mysql;
    vector<string> vec;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}
