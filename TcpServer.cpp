#include "TcpServer.h"

TcpServer::TcpServer(const std::string &ip,const uint16_t port,int threadnum)
:threadnum_(threadnum),mainloop_(new EventLoop(true)),acceptor_(mainloop_.get(),ip,port)
,threadpool_(threadnum_,"IO")
{
    // acceptor_=new Acceptor(mainloop_,ip,port);
    acceptor_.setnewconnCB(std::bind(&TcpServer::newconnection,this,std::placeholders::_1));
    mainloop_->setEpollTimeoutCB(std::bind(&TcpServer::epolltimeout,this,std::placeholders::_1));

    //创建线程池
    // threadpool_=new ThreadPool(threadnum_,"IO");

    //创建从事件循环
    for(int i=0;i<threadnum_;++i){
        subloops_.emplace_back(new EventLoop(false,5,10));
        subloops_[i]->setEpollTimeoutCB(std::bind(&TcpServer::epolltimeout,this,std::placeholders::_1));
        subloops_[i]->setremoveconnCB(std::bind(&TcpServer::removeconn,this,std::placeholders::_1));
        threadpool_.addtask(std::bind(&EventLoop::run,subloops_[i].get()));//将事件循环的run函数添加到任务队列中
        sleep(1);
    }

}

TcpServer::~TcpServer()
{
    // delete acceptor_;
    // delete mainloop_;
    //释放全部连接
    // for(auto& conn:conns_){
    //     delete conn.second;
    // }
    //释放全部从事件循环
    // for(auto& loop:subloops_){
    //     delete loop;
    // }
    // delete threadpool_;
}

void TcpServer::start()
{
    mainloop_->run();//运行事件循环
}
//处理客户端连接请求
void TcpServer::newconnection(std::unique_ptr<Socket> clientsock){
    //将新建连接放在从循环中
    spConnection conn(new Connection(subloops_[clientsock->fd()%threadnum_].get(),std::move(clientsock)));
    conn->setCloseConnCB(std::bind(&TcpServer::closeconnection,this,std::placeholders::_1));
    conn->setErrorConnCB(std::bind(&TcpServer::errorconnection,this,std::placeholders::_1));
    conn->setOnmessageCB(std::bind(&TcpServer::onmessage,this,std::placeholders::_1,std::placeholders::_2));
    conn->setSendComplete(std::bind(&TcpServer::sendComplete,this,std::placeholders::_1));
    
    {
        std::lock_guard<std::mutex> locak(mmutex_);
        conns_[conn->fd()]=conn;
    }
    // printf ("new connection(fd=%d,ip=%s,port=%d) ok.\n",conn->fd(),conn->ip().c_str(),conn->port());
    subloops_[conn->fd()%threadnum_]->newconnection(conn);
    newConnCB_(conn);
}
//处理客户端close
void TcpServer::closeconnection(spConnection conn)
{
    closeConnCB_(conn);
    // printf("client(eventfd=%d) disconnected.\n",conn->fd());
    //close(conn->fd()); //析构conn--》析构sock--》关闭fd，因此不需要手动关闭
    {
        std::lock_guard<std::mutex> locak(mmutex_);
        conns_.erase(conn->fd());
    }
    //delete conn;//析构conn--》析构sock--》关闭fd
}
//处理客户端error
void TcpServer::errorconnection(spConnection conn)
{
    errorConnCB_(conn);
    // printf("client(eventfd=%d) disconnected.\n",conn->fd());
    // close(conn->fd());            // 关闭客户端的fd。
    {
        std::lock_guard<std::mutex> locak(mmutex_);
        conns_.erase(conn->fd());
    }
    //delete conn;//析构conn--》析构sock--》关闭fd
}
//处理接收到的数据
void TcpServer::onmessage(spConnection conn,std::string message)
{
    onMessageCB_(conn,message);
}
//数据发送完毕，Connection回调此函数通知TcpServer
void TcpServer::sendComplete(spConnection conn)
{
    // printf("send complete\n");
    sendCompleteCB_(conn);
    //根据业务需求，执行相应业务
}
//超时
void TcpServer::epolltimeout(EventLoop* loop)
{
    // printf("wait timeout\n");
    timeoutCB_(loop);
}

void TcpServer::setNewConnCB(std::function<void(spConnection)> fn)
{
    newConnCB_=fn;
}
void TcpServer::setCloseConnCB(std::function<void(spConnection)> fn)
{
    closeConnCB_=fn;
}
void TcpServer::setErrorConnCB(std::function<void(spConnection)> fn)
{
    errorConnCB_=fn;
}
void TcpServer::setOnMessageCB(std::function<void(spConnection,std::string &message)> fn)
{
    onMessageCB_=fn;
}
void TcpServer::setSendCompleteCB(std::function<void(spConnection)> fn)
{
    sendCompleteCB_=fn;
}
void TcpServer::setTimeoutCB(std::function<void(EventLoop*)> fn)
{
    timeoutCB_=fn;
}
//删除链接
void TcpServer::removeconn(int fd)
{
    {
        std::lock_guard<std::mutex> locak(mmutex_);
        conns_.erase(fd);
    }
}
void TcpServer::stop()
{
    mainloop_->stop();
    std::cout<<"mainloop stop"<<std::endl;
    for(int i=0;i<threadnum_;++i){
        subloops_[i]->stop();
    }
    std::cout<<"subloops_ stop"<<std::endl;
    threadpool_.stop();
    std::cout<<"IO threadpool_ stop"<<std::endl;
}