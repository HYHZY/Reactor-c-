#pragma once
#include "EventLoop.h"
#include "Channel.h"
#include "Acceptor.h"
#include "Connection.h"
#include <functional>
#include <map>
#include "ThreadPool.h"
#include <memory>
#include <mutex>
class TcpServer{
private:
    std::unique_ptr<EventLoop> mainloop_;//主事件循环，可用堆内存或栈内存
    Acceptor acceptor_;//一个tcpserver只有一个acceptor
    //创建conn在主事件循环，释放连接在从事件循环，因此需要对此加锁
    std::map<int,spConnection> conns_;//一个TCPSERVER有多个连接
    std::mutex mmutex_;//
    std::vector<std::unique_ptr<EventLoop>> subloops_;//从事件循环，只能用堆内存
    int threadnum_;//线程池大小
    ThreadPool threadpool_;//线程池
    

    std::function<void(spConnection)> newConnCB_;
    std::function<void(spConnection)> closeConnCB_;
    std::function<void(spConnection)> errorConnCB_;
    std::function<void(spConnection,std::string& message)> onMessageCB_;
    std::function<void(spConnection)> sendCompleteCB_;
    std::function<void(EventLoop*)> timeoutCB_;
    
    
public:
    TcpServer(const std::string &ip,const uint16_t port,int threadnum=3);
    ~TcpServer();
    void start();
    void stop();

    void onmessage(spConnection conn,std::string message);//处理接收到的数据
    //connection对象属于tcpserver
    void newconnection(std::unique_ptr<Socket> clientsock);//处理客户端连接请求
    void closeconnection(spConnection conn);//处理客户端close
    void errorconnection(spConnection conn);//处理客户端error

    void sendComplete(spConnection conn);//数据发送完毕，Connection回调此函数通知TcpServer
    void epolltimeout(EventLoop* loop);//超时

    void setNewConnCB(std::function<void(spConnection)> fn);
    void setCloseConnCB(std::function<void(spConnection)> fn);
    void setErrorConnCB(std::function<void(spConnection)> fn);
    void setOnMessageCB(std::function<void(spConnection,std::string &message)> fn);
    void setSendCompleteCB(std::function<void(spConnection)> fn);
    void setTimeoutCB(std::function<void(EventLoop*)> fn);

    void removeconn(int fd);//删除链接
};