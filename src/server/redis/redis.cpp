#include "redis.hpp"
#include <iostream>


Redis::Redis()
    : _publish_contex(nullptr), _subcribe_contex(nullptr)
{
}

Redis::~Redis()
{
    if (_publish_contex != nullptr) {
        redisFree(_publish_contex);
    }
    if (_subcribe_contex != nullptr) {
        redisFree(_subcribe_contex);
    }
}

bool Redis::connect() {
    _publish_contex = redisConnect("127.0.0.1",6379);
    if (nullptr == _publish_contex) {
        std::cerr << "connect redis failed!" << std::endl;
        return false;
    }
    _subcribe_contex = redisConnect("127.0.0.1",6379);
    if (nullptr == _subcribe_contex) {
        std::cerr << "connect redis failed!" << std::endl;
        return false;
    }
    // 在单独的线程中，监听通道上的事件 有消息给业务层进行上报
    std::thread t([&](){
        observer_channel_message();
    });
    t.detach();

    std::cout << "connect redis-server success!" << std::endl;
    return true;
}

bool Redis::publish(int channel,std::string message) {
    redisReply *reply = (redisReply *)redisCommand(_publish_contex,"PUBLISH %d %s",
    channel,message.c_str());
    if (nullptr == reply) {
        std::cerr << "publish command failed!" << std::endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool Redis::subscribe(int channel) {
    // subscribe本身会造成线程阻塞等待通道里面发生消息 这里只做订阅通道 不接受通道消息
    // 通道消息的接收专门在observer_channel_message函数中的独立线程中进行
    // 只负责发送命令 不阻塞接收redis server响应消息 否责和notifyMsg线程抢占资源
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_contex,"SUBSCRIBE %d",channel)) {
        std::cerr << "subscribe command failed!" << std::endl;
        return false;
    }
    // redisbufferwrite可以循环发送缓冲区 直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_contex,&done)) {
            std::cerr << "subscribe command failed!" << std::endl;
            return false;
        }
    }
    return true;
}

bool Redis::unsubscribe(int channel) {
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_contex,"UNSUBSCRIBE %d",channel)){
        std::cerr << "unsubscribe command failed!" << std::endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1)
    int done = 0;
    while(!done) {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_contex, &done))
        {
            std::cerr << "unsubscribe command failed!" << std::endl;
            return false;
        }
    }
    return true;
}

// 在独立线程中接收订阅通道中的消息
void Redis::observer_channel_message() {
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(this->_subcribe_contex,(void **)&reply)) {
        // 订阅收到的消息是一个带三元素的数组
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr) {
            _notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
    std::cerr << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<" << std::endl;
}

void Redis::init_notify_handler(std::function<void(int,std::string)> fn)
{
    this->_notify_message_handler = fn;
}