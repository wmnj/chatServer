#ifndef GROUPUSER_H
#define GROUPUSER_H
#include "user.hpp"
#include "public.hpp"

class GroupUser : public User
{
private:
    string role; // creator normal
public:
    GroupUser(int gid = -1, string role = "normal") {
        this->role = role;
    }
    void setRole(string role) {this->role = role;}
    string getRole() {return this->role;}
};


#endif
