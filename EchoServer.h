#pragma once
#include "TcpServer.h"
#include "EventLoop.h"
#include "Connection.h"
#include "ThreadPool.h"

class EchoServer{
private:
    TcpServer tcpserver_;
    ThreadPool threadpool_;//工作线程池
public:
    //增加工作线程，计算+IO，计算会阻塞事件循环，要放在工作线程中
    EchoServer(const std::string &ip,const uint16_t port,int subthreadnum=3,int workthreadnum=5);
    ~EchoServer();

    void start();
    void stop();

    void HandleMessage(spConnection conn,std::string& message);//处理接收到的数据
    //connection对象属于tcpserver
    void HandleNewConn(spConnection conn);//处理客户端连接请求
    void HandleCloseConn(spConnection conn);//处理客户端close
    void HandleErrorConn(spConnection conn);//处理客户端error
    void HandleSendComplete(spConnection conn);//数据发送完毕，Connection回调此函数通知TcpServer
    void HandleTimeout(EventLoop* loop);//超时

    //业务处理函数
    void OnMessage(spConnection conn,std::string& message);
};