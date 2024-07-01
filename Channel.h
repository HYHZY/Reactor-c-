#pragma once
#include <sys/epoll.h>
#include <functional>
#include "InetAddress.h"
#include "Socket.h"
#include "comFun.h"
#include "EventLoop.h"
#include <memory>

class EventLoop;

class Channel{
private:
    int fd_=-1;//Channel类，channel和fd一一对应
    // Epoll *ep_=nullptr;//channel的红黑树，channel与epoll是多对一的关系，一个channel对应一个epoll
    EventLoop* loop_=nullptr;// Channel对应的事件循环，Channel与EventLoop是多对一的关系，一个Channel只对应一个EventLoop。
    bool inepoll_=false;//channel是否已经添加到epoll树上，如果未添加，调用epoll_ctl的时候用EPOLL_CTL_ADD，否则用EPOLL_CTL_MOD
    uint32_t events_=0;//fd_需要监视的事件
    uint32_t revents_=0;//fd_已发生的事件

    std::function<void()> readcallback_;//fd_读事件的回调函数
    std::function<void()> closecallback_;//fd_关闭的回调函数 ，回调Connection的closeCB()
    std::function<void()> errorcallback_;//fd_错误事件的回调函数，回调Connection的errorCB()
    std::function<void()> writecallback_;//fd_错误事件的回调函数，回调Connection的writeCB()

public:
    Channel(EventLoop* loop, int fd);
    ~Channel();

    int fd();
    
    void useET();//边沿触发
    void enablereading();//注册fd_读事件
    void disabllereading();//取消读事件
    void enablewriting();//注册fd_写事件
    void disabllewriting();//取消写事件
    //取消全部事件，因为tcp来凝结断开和connection析构之间有时间差，不应在关注任务事件
    void disable();
    //从事件循环中删除channel
    void remove();

    void setinepoll();//设置inepoll_为true
    void setrevents(uint32_t ev);
    bool ineopll();//判断fd是否在epoll中管理
    uint32_t events();
    uint32_t revents();

    void handleevent();//事件处理函数
    
    void setreadcallback(std::function<void()> fn);
    void seterrorcallback(std::function<void()> fn);
    void setclosecallback(std::function<void()> fn);
    void setwritecallback(std::function<void()> fn);
};