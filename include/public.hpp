#ifndef PUBLIC_H
#define PUBLIC_H

enum EnMsgType {
    LOGIN_MSG = 1, // 登录
    LOGIN_MSG_ACK = 2,
    REG_MSG = 3, // 注册
    REG_MSG_ACK = 4, 
    ONE_CHAT_MSG = 5, // 给好友发一次消息
    ADD_FRIEND_MSG = 6, // 添加好友

    CREATE_GROUP_MSG = 7, // 创建群组
    ADD_GROUP_MSG = 8, // 加群
    GROUP_CHAT_MSG = 9, // 群聊天
    LOGINOUT_MSG = 10,
};


#endif