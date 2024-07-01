#pragma once
#include <functional>
#include "Socket.h"
#include "InetAddress.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Buffer.h"
#include <memory>
#include <atomic>
#include "Timestamp.h"
class EventLoop;
class Channel;
class Connection;//前置声明
using spConnection=std::shared_ptr<Connection>;
class Connection:public std::enable_shared_from_this<Connection>{
private:
    EventLoop* loop_;
    std::unique_ptr<Socket> clientsock_;//生命周期由connection管理
    std::unique_ptr<Channel> clientch_;
    Buffer inputbuf_;
    Buffer outputbuf_;
    std::atomic_bool disconnect_;//客户端连接是否断开

    std::function<void(spConnection)> closecallback_;//fd_关闭的回调函数 ，回调tcpserver的closeconnection()
    std::function<void(spConnection)> errorcallback_;//fd_错误事件的回调函数，回调tcpserver的errorconnection()
    std::function<void(spConnection,std::string&)> onmessageCB_;//数据处理回调函数，回调tcpserver的onmessage()
    std::function<void(spConnection)> sendCompleteCB_;//数据发送完毕回调函数，回调tcpserver的sendComplete()

    Timestamp lasttime_;//时间戳，创建connection对象为当前时间，每接收一个报文，更新时间戳
public:
    Connection(EventLoop* loop,std::unique_ptr<Socket> clientsock);
    ~Connection();

    int fd()const;
    std::string ip()const;
    uint16_t port()const;

    void closeCB();//tcp断开
    void errorCB();//tcp错误
    void onmessage();//处理对端发过来的消息
    void writeCB();//写事件回调函数，供channel回调
    //发送数据,不管在任何线程中，都调用此函数发送数据
    void Send(const char* data,size_t size);
    //发送数据,IO线程直接调用此函数，work线程将此函数传给IO线程
    void Sendinloop(std::string &data,size_t size);

    void setCloseConnCB(std::function<void(spConnection)> fn);
    void setErrorConnCB(std::function<void(spConnection)> fn);
    void setOnmessageCB(std::function<void(spConnection,std::string&)> fn);
    void setSendComplete(std::function<void(spConnection)> fn);

    bool timeout(time_t now,int val);
};