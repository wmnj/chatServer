#include "usermodel.hpp"
#include "db.hpp"
#include <muduo/base/Logging.h>

bool UserModel::insert(User &user)
{
    // 获取数据库连接
    char sql[1024] = {0};
    sprintf(sql, "insert into User (name, password, state) values ('%s','%s','%s')", user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取id存入对象中
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

// 根据用户号码查询用户信息
User UserModel::query(int id)
{
    char sql[1024];
    sprintf(sql, "select * from User where id = %d", id);
    MySQL mysql;
    User user;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                // id name password state
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
            }
        }       
    }
    return user;
}

// 更新用户的状态信息
bool UserModel::updateState(User user) {
    MySQL mysql;
    if(mysql.connect()) {
        char sql[1024];
        sprintf(sql,"update User set state = '%s' where id = %d",user.getState().c_str(),user.getId());
        if(mysql.update(sql)) {
            return true;
        }
    }
    return false;
}

void UserModel::resetState() {
    MySQL mysql;
    if(mysql.connect()) {
        char sql[100] = "update User set state = 'offline' where state = 'online'";
        mysql.update(sql);
    }
}