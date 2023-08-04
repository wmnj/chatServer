#include "db.hpp"
#include <muduo/base/Logging.h>

static string server = "127.0.0.1";
static string user = "root";
static string pwd = "rootpass";
static string dname = "chat";

MySQL::MySQL()
{
    MySQL::_conn = mysql_init(nullptr);
}

MySQL::~MySQL()
{
    if (_conn != nullptr)
    {
        mysql_close(_conn);
    }
}

bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(), pwd.c_str(), dname.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        mysql_query(_conn, "set names gbk");
        LOG_INFO << "connect mysql sucess!";
    }
    else
    {
        LOG_INFO << "connect mysql fail.";
    }
    return p;
}

MYSQL *MySQL::getConnection()
{
    return _conn;
}

MYSQL_RES *MySQL::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << " query fail!";
        return nullptr;
    }
    return mysql_use_result(_conn);
}

bool MySQL::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << " update fail!";
        return false;
    }
    return true;
}