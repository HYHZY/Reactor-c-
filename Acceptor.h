#pragma once
#include <functional>
#include "Socket.h"
#include "InetAddress.h"
#include "EventLoop.h"
#include "Channel.h"
#include <memory>

class Acceptor{
private:
    EventLoop* loop_;//没有所有权,只能用常引用,不能用移动语义
    Socket servsock_;//占用内存很小，用栈内存
    Channel acceptch_;//占用内存很小，用栈内存

    std::function<void(std::unique_ptr<Socket> clientsock)> newconnCB_;//创建客户端连接回调函数
public:
    Acceptor(EventLoop* loop,const std::string &ip,const uint16_t port);
    ~Acceptor();

    void newconnection();//处理客户端连接请求
    void setnewconnCB(std::function<void(std::unique_ptr<Socket> clientsock)> fn);
};