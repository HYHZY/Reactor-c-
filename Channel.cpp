#include "Channel.h"

Channel::Channel(EventLoop* loop,int fd)
:loop_(loop),fd_(fd)
{
}

Channel::~Channel()
{ 
    //Channel不能关闭ep，ep不属于channel，进近使用而已
    //delete ep_;
}

int Channel::fd()
{
    return fd_;
}
void Channel::useET()//边沿触发
{
    events_|=EPOLLET;
}
//注册fd_读事件
void Channel::enablereading()
{
    events_|=EPOLLIN;
    loop_->updatechannel(this);
}
//取消读事件
void Channel::disabllereading()
{
    events_ &= ~EPOLLIN;
    loop_->updatechannel(this);
} 
//注册fd_写事件
void Channel::enablewriting()
{
    events_|=EPOLLOUT;
    loop_->updatechannel(this);
}
//取消写事件
void Channel::disabllewriting()
{
    events_&=~EPOLLOUT;
    loop_->updatechannel(this);
}
void Channel::setinepoll()//设置inepoll_为true
{
    inepoll_=true;
}
void Channel::setrevents(uint32_t ev)
{
    revents_=ev;
}

//取消全部事件，因为tcp来凝结断开和connection析构之间有时间差，不应在关注任务事件
void Channel::disable()
{
    events_=0;
    loop_->updatechannel(this);
}
//从事件循环中删除channel
void Channel::remove()
{
    disable();  //可以不写
    loop_->removechannel(this);//从rbt删除fd
}
bool Channel::ineopll()
{
    return inepoll_;
}
uint32_t Channel::events()
{
    return events_;
}
uint32_t Channel::revents()
{
    return revents_;
}

void Channel::handleevent(){
    if (revents_ & EPOLLRDHUP)                     // 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
    {
        // std::cout<<"EPOLLRDHUP"<<std::endl;;
        closecallback_();
    }                                //  普通数据  带外数据
    else if (revents_ & (EPOLLIN|EPOLLPRI))   // 接收缓冲区中有数据可以读。
    {
        // std::cout<<"EPOLLIN|EPOLLPRI"<<std::endl;;
        readcallback_();//实现不同channel绑定不同的函数，类似于动态多态
    }
    else if (revents_ & EPOLLOUT)    // 有数据需要写，暂时没有代码，以后再说。
    {
        // std::cout<<"EPOLLOUT"<<std::endl;;
        writecallback_();
    }
    else // 其它事件，都视为错误。
    {
        errorcallback_();
    }
}

void Channel::setreadcallback(std::function<void()> fn){
    readcallback_=fn;
}

void Channel::seterrorcallback(std::function<void()> fn)
{
    errorcallback_=fn;
}
void Channel::setclosecallback(std::function<void()> fn)
{
    closecallback_=fn;
}
void Channel::setwritecallback(std::function<void()> fn)
{
    writecallback_=fn;
}