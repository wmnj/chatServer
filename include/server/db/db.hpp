#ifndef DB_H
#define DB_H
#include <mysql/mysql.h>
#include <string>
using namespace std;

class MySQL
{
private:
    MYSQL *_conn;
public:
    MySQL();
    ~MySQL();
    bool connect();
    bool update(string sql);
    MYSQL_RES *query(string sql);
    MYSQL *getConnection();
};



#endif