#include "Acceptor.h"

Acceptor::Acceptor(EventLoop* loop,const std::string &ip,const uint16_t port)
:loop_(loop),servsock_(createnonblocking()),acceptch_(loop_,servsock_.fd())
{
    InetAddress servaddr(ip,port);
    servsock_.setreuseaddr(true);
    servsock_.setreuseport(true);
    servsock_.settcpnodelay(true);
    servsock_.setkeepalive(true);
    servsock_.Bind(servaddr);
    servsock_.Listen();
    
    acceptch_.setreadcallback(std::bind(&Acceptor::newconnection,this));
    acceptch_.enablereading(); 
}
Acceptor::~Acceptor()
{
    
}
#include "Connection.h"
//处理客户端连接请求
void Acceptor::newconnection()
{
    InetAddress clientaddr;
    //sock只能new，否则析构函数会关闭fd
    std::unique_ptr<Socket> clientsock(new Socket(servsock_.accept(clientaddr)));
    clientsock->setipport(clientaddr.ip(),clientaddr.port());
    // 为新客户端连接准备读事件，并添加到epoll中。
    // Connection *conn=new Connection(loop_,clientsock);
    newconnCB_(std::move(clientsock));//要使用移动语义
}

void Acceptor::setnewconnCB(std::function<void(std::unique_ptr<Socket> clientsock)> fn)
{
    newconnCB_=fn;
}