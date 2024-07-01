#pragma once
#include "Epoll.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <memory>
#include <queue>
#include <mutex>
#include <atomic>
#include <sys/eventfd.h>
#include <sys/timerfd.h>//定时器fd
#include <functional>
#include "Timestamp.h"
#include <map>
#include "Connection.h"

class Channel;
class Epoll;
class Connection;

using spConnection=std::shared_ptr<Connection>;
class EventLoop{
private:
    int timeval_;//闹钟间隔，秒
    int timeout_;//超时时间

    std::unique_ptr<Epoll> ep_;//每个事件循环只有一个Epoll
    std::function<void(EventLoop*)> epolltimeoutCB_;//超时回调函数
    pid_t threadid_;//事件循环所在的线程id

    std::queue<std::function<void()>> taskqueue_;//事件循环线程被eventfd唤醒后执行的任务队列。
    std::mutex mutex_;
    //使用eventfd异步唤醒IO线程，同时将eventfd添加到rbt中管理
    int wakeupfd_;// 用于唤醒事件循环线程的eventfd。
    std::unique_ptr<Channel> wakeupch_;// eventfd的Channel。
    
    int timerfd_;//定时器
    std::unique_ptr<Channel> timerch_;
    bool mainloop_;//区分主从事件循环 
    //1.事件循环增加map<fd,connection> conns_,存放运行在该事件循环上的所有conn
    //2.定时器响了，遍历conns_,将超时conn删除，同时要删除tcpserver的conns_中的conn
    //3.TCPserver和Eventloop需要加锁
    //4.闹钟时间间隔和超时时间参数化

    std::mutex mmutex_;//
    std::map<int,spConnection> conns_;//存放事件循环所有conn
    std::function<void(int)> removeconnCB_;//删除Tcpserver.conn_中的conn，回调Tcpserver::removeconn

    std::atomic_bool stop_;//退出事件循环标志
public:
    EventLoop(bool mainloop,int timeval=30,int timeout=80);//构造函数创建Epoll
    ~EventLoop();//销毁Epoll

    void run();//运行事件循环
    void stop();//退出事件循环
    // Epoll* ep();
    void updatechannel(Channel* ch);// 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
    void setEpollTimeoutCB(std::function<void(EventLoop*)> fn);
    void removechannel(Channel* ch);//从事件循环删除channel
    bool isInLoopthread();//判断当前线程是否为事件循环线程

    void queueinloop(std::function<void()> fn);// 把任务添加到队列中。

    void wakeup();// 用eventfd唤醒事件循环线程。
    void handlewakeup();//事件循环被唤醒执行的函数

    void handletimer();

    void newconnection(spConnection conn);
    void setremoveconnCB(std::function<void(int)> fn);
};