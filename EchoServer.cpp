#include "EchoServer.h"
using namespace std;
EchoServer::EchoServer(const std::string &ip,const uint16_t port,int subthreadnum,int workthreadnum)
:tcpserver_(ip,port,subthreadnum),threadpool_(workthreadnum,"WORK")
{
    tcpserver_.setNewConnCB(std::bind(&EchoServer::HandleNewConn,this,std::placeholders::_1));
    tcpserver_.setCloseConnCB(std::bind(&EchoServer::HandleCloseConn,this,std::placeholders::_1));
    tcpserver_.setErrorConnCB(std::bind(&EchoServer::HandleErrorConn,this,std::placeholders::_1));
    tcpserver_.setOnMessageCB(std::bind(&EchoServer::HandleMessage,this,std::placeholders::_1,std::placeholders::_2));
    tcpserver_.setSendCompleteCB(std::bind(&EchoServer::HandleSendComplete,this,std::placeholders::_1));
    tcpserver_.setTimeoutCB(std::bind(&EchoServer::HandleTimeout,this,std::placeholders::_1));
}
EchoServer::~EchoServer()
{
    stop();
}

void EchoServer::start()
{
    tcpserver_.start();
}
void EchoServer::stop()
{
    threadpool_.stop();//工作线程停止，echoserver对象
    std::cout<<"WORK threadpool_ stop"<<std::endl;
    tcpserver_.stop();
}
//处理接收到的数据
void EchoServer::HandleMessage(spConnection conn,std::string& message)
{
    // printf("Echoserver::Handlemessage in thread(%ld)\n",syscall(SYS_gettid));
    if(threadpool_.size()==0){
        OnMessage(conn,message);
    }
    else{
        threadpool_.addtask(std::bind(&EchoServer::OnMessage,this,conn,message));
    }
}
//业务处理
void EchoServer::OnMessage(spConnection conn,std::string& message)
{
    //业务
    message="reply: "+message;//回显业务
    // cout<<"业务处理"<<endl;
    conn->Send(message.data(),message.size());//发送数据
}
////处理客户端连接请求
void EchoServer::HandleNewConn(spConnection conn)
{
    printf("%s Echoserver::HandleNewConn in thread(%ld)\n",Timestamp::now().tostring().c_str(),syscall(SYS_gettid));
}
//处理客户端close
void EchoServer::HandleCloseConn(spConnection conn)
{
    printf("%s Echoserver::HandleCloseConn in thread(%ld)\n",Timestamp::now().tostring().c_str(),syscall(SYS_gettid));
    //业务
}
//处理客户端error
void EchoServer::HandleErrorConn(spConnection conn)
{
    // cout<<"Connection error"<<endl;
    //业务
}
//数据发送完毕
void EchoServer::HandleSendComplete(spConnection conn)
{
    // cout<<"Message send complete"<<endl;
    //业务
    
}
//超时
void EchoServer::HandleTimeout(EventLoop* loop)
{
    // cout<<"EchoServer timeout"<<endl;
    //业务
}