#include "Connection.h"

Connection::Connection(EventLoop* loop,std::unique_ptr<Socket> clientsock)
:loop_(loop),clientsock_(std::move(clientsock)),disconnect_(false)
,clientch_(new Channel(loop_,clientsock_->fd()))
{
    //太耻辱了，明明是类的成员变量，我非要重新定义一个局部变量，导致我的clientch_未初始化，
    //引以为戒，gdb调试总是报错，段错误，未初始化，tmd，给自己两耳屎
    // Channel *clientch_=new Channel(loop_,clientsock->fd());
   
    // 为新客户端连接准备读事件，并添加到epoll中。
    // clientch_=new Channel(loop_,clientsock_->fd());//这里原本使用clientsock，段错误，因为移动语义，打自己！！！
    //对fd的读写不应该放在channel中，应该在connection统一管理，channel可以回调
    clientch_->setreadcallback(std::bind(&Connection::onmessage,this));
    clientch_->setclosecallback(std::bind(&Connection::closeCB,this));
    clientch_->seterrorcallback(std::bind(&Connection::errorCB,this));
    clientch_->setwritecallback(std::bind(&Connection::writeCB,this));
     
    clientch_->useET();
    clientch_->enablereading(); 
}
Connection::~Connection()
{
    // delete clientsock_;//生命周期与connection一样
    // delete clientch_;
    std::cout<<"connection 析构"<<std::endl;
}

int Connection::fd()const{
    return clientsock_->fd();
}
std::string Connection::ip()const
{
    return clientsock_->ip();
}
uint16_t Connection::port()const
{
    return clientsock_->port();
}
//tcp断开
void Connection::closeCB()
{
    disconnect_=true;
    clientch_->remove();//删除channel
    closecallback_(shared_from_this());
}
//tcp错误
void Connection::errorCB()
{
    disconnect_=true;
    clientch_->remove();//删除channel
    errorcallback_(shared_from_this());
}

void Connection::setCloseConnCB(std::function<void(spConnection)> fn)
{
    closecallback_=fn;
}
void Connection::setErrorConnCB(std::function<void(spConnection)> fn)
{
    errorcallback_=fn;
}
//处理对端发过来的消息
void Connection::onmessage()
{
    char buffer[BUFSIZE];
    while (true)             // 由于使用非阻塞IO，一次读取buffer大小数据，直到全部的数据读取完毕。
    {    
        bzero(&buffer, sizeof(buffer));
        ssize_t nread = read(fd(), buffer, sizeof(buffer));    // 这行代码用了read()，也可以用recv()，一样的，不要纠结。
        if (nread > 0)      // 成功的读取到了数据。
        {
            // 把接收到的报文内容原封不动的发回去。
            // printf("recv(chentfd=%d):%s\n",fd(),buffer);
            // send(fd(),buffer,strlen(buffer),0);
            inputbuf_.append(buffer,nread);
        } 
        else if (nread == -1 && errno == EINTR) // 读取数据的时候被信号中断，继续读取。
        {  
            continue;
        } 
        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 全部的数据已读取完毕。
        {
            std::string message;
            while(1){
                if(inputbuf_.pickmessage(message)==false) break;
                printf("message (clientfd=%d):%s\n",fd(),message.c_str());
                
                lasttime_=Timestamp::now();//更新时间戳
                //进行运算
                onmessageCB_(shared_from_this(),message);
            }
            
            break;
        } 
        else if (nread == 0)  // 客户端连接已断开。
        {  
            closeCB();
            break;
        }
    }
}

void Connection::setOnmessageCB(std::function<void(spConnection,std::string&)> fn)
{
    onmessageCB_=fn;
}
//发送数据
void Connection::Send(const char* data,size_t size)
{
    if(disconnect_){
        // printf("disconnect send return  directly\n");
        return;
    }
    //用string传参，不用const char*，不知道为啥课程可以，我的不行
    //局部变量会被释放掉,但是string有复制功能，写时复制，复制只增加引用计数，只有在修改时间才实际复制
    std::string message(data);
    if(loop_->isInLoopthread()){
        // printf("send() not in Eventloop\n");
        Sendinloop(message,size);
    }
    else{
        // printf("send() in Eventloop data:%s size=%d\n",data,size);
        loop_->queueinloop(std::bind(&Connection::Sendinloop,this,message,size));
    }
}
//发送数据,IO线程直接调用此函数，work线程将此函数传给IO线程
//用std::string有自己的内存管理，用const char*,在回调传参的时候无法传递，指针data的内容会被释放
void Connection::Sendinloop(std::string &message,size_t size){
    // printf("send() in Sendinloop data:%s size=%d\n",message.data(),size);
    outputbuf_.appendwithseq(message.data(),size);
    // printf("send() in Sendinloop outputbuf_.data:%s size=%d\n",outputbuf_.data()+4,outputbuf_.size());
    //注册写事件
    clientch_->enablewriting();//注册fd_写事件
}
//写事件回调函数，供channel回调
void Connection::writeCB()
{
    // printf("send() in writeCB data:%s size=%d\n",outputbuf_.data(),outputbuf_.size());
    int writen=::send(fd(),outputbuf_.data(),outputbuf_.size(),0);
    if(writen>0)//清除发送数据
    {
        outputbuf_.erase(0,writen);
    }
    if(outputbuf_.size()==0){
        clientch_->disabllewriting();
        sendCompleteCB_(shared_from_this());
    }
}
//数据发送完毕回调函数，回调tcpserver的sendComplete(
void Connection::setSendComplete(std::function<void(spConnection)> fn)
{
    sendCompleteCB_=fn;
}

bool Connection::timeout(time_t now,int val)
{
    return now-lasttime_.toint()>val;
}