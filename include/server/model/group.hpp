#ifndef GROUP_H
#define GROUP_H

#include <string>
#include <vector>
#include "groupuser.hpp"

class Group
{
private:
    int id;
    string name;
    string desc;
    vector<GroupUser> users;
public:
    Group(int id = -1, string name="", string desc=""){
        this->id = id;
        this->name = name;
        this->desc = desc;
    }
    void setGroupId(int gid) {this->id = gid;}
    void setGroupName(string name) {this->name = name;}
    void setDesc(string desc) {this->desc = desc;}
    int getGroupId() {return this->id;}
    string getGroupName() {return this->name;}
    string getDesc() {return this->desc;}
    vector<GroupUser>  getUsers() {return this->users;}
};

#endif