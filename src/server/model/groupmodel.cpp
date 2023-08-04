#include "db.hpp"
#include "groupmodel.hpp"

//  新建组
bool GroupModel::insert(Group &group)
{
    string name = group.getGroupName();
    string desc = group.getDesc();
    MySQL mysql;
    if (mysql.connect())
    {
        char sql[1024];
        sprintf(sql, "insert into AllGroup(groupname,groupdesc) values('%s','%s')", name.c_str(), desc.c_str());
        if (mysql.update(sql))
        {
            group.setGroupId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

// 加入群组
void GroupModel::addGroup(int userid, int groupid, string role)
{
    // 插入到groupuser表中
    MySQL mysql;
    if (mysql.connect())
    {
        char sql[1024] = {0};
        sprintf(sql, "insert into GroupUser(groupid,userid,grouprole) values(%d, %d,'%s')",
                groupid, userid, role.c_str());
        mysql.update(sql);    
    }
}

// 查询用户所在群组信息
vector<Group> GroupModel::queryGroups(int userid)
{
    /*
    1. 先根据userid在groupuser表中查询出该用户所属的群组信息
    2. 在根据群组信息，查询属于该群组的所有用户的userid，并且和user表进行多表联合查询，查出用户的详细信息
    */
    // 联合查询all group和groupuser获取组信息
    char sql[1024];
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from GroupUser b inner join AllGroup a on b.groupid = a.id where b.userid = %d", userid);
    MySQL mysql;
    vector<Group> vec;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                // id name password state
                Group group;
                group.setGroupId(atoi(row[0]));
                group.setGroupName(row[1]);
                group.setDesc(row[2]);
                vec.push_back(group);
            }
            mysql_free_result(res);
        }
    }
    // 2. 在根据群组信息，查询属于该群组的所有用户的userid，并且和user表进行多表联合查询，查出用户的详细信息
    for (auto &gup : vec)
    {
        int gid = gup.getGroupId();
        if (mysql.connect())
        {
            sprintf(sql, "select a.id,a.name,a.state,b.grouprole from User a inner join GroupUser b on b.userid = a.id where b.groupid = %d", gid);
            MYSQL_RES *res = mysql.query(sql);
            // 获取群组中的用户信息
            if (res != nullptr)
            {
                MYSQL_ROW row;               
                while ((row = mysql_fetch_row(res)) != nullptr)
                {
                    // id name password state
                    GroupUser usr;
                    usr.setId(atoi(row[0]));
                    usr.setName(row[1]);
                    usr.setState(row[2]);
                    usr.setRole(row[3]);
                    gup.getUsers().push_back(usr);
                }
                mysql_free_result(res);
            }
        }
    }
    return vec;
}

// 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其它成员群发消息
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{ //    查询groupuser
    char sql[1024];
    // 使用联合索引
    sprintf(sql, "select userid from GroupUser where groupid = %d and userid != %d", groupid, userid);
    MySQL mysql;
    vector<int> vec;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return vec;
}
