#include "Epoll.h"

Epoll::Epoll(){
    epollfd_=epoll_create(1);
    if(epollfd_==-1){
        printf("epoll create faild(%d)\n",errno);
        exit(-1);
    }
}
Epoll::~Epoll(){
    close(epollfd_);
}
/*
void Epoll::addfd(int fd,uint32_t op){
    // 为服务端的listenfd准备读事件。
    struct epoll_event ev;              // 声明事件的数据结构。
    ev.data.fd=fd;                 // 指定事件的自定义数据，会随着epoll_wait()返回的事件一并返回。
    ev.events=op;                // 让epoll监视listenfd的读事件，采用水平触发。

    // 把需要监视的listenfd和它的事件加入epollfd中。
    if(epoll_ctl(epollfd_,EPOLL_CTL_ADD,fd,&ev)==-1){
        printf("epoll ctl faild(%d)\n",errno);
        exit(-1);
    }
}
*/
void Epoll::updatechannel(Channel* ch)
{
    epoll_event ev;
    ev.data.ptr=ch;
    ev.events=ch->events();

    if(ch->ineopll()){
        if(epoll_ctl(epollfd_,EPOLL_CTL_MOD,ch->fd(),&ev)==-1)
        {
            perror("epoll mod falid");
            exit(-1);
        }
    }
    else
    {
        if(epoll_ctl(epollfd_,EPOLL_CTL_ADD,ch->fd(),&ev)==-1)
        {
            perror("epoll add falid");
            exit(-1);
        }
        ch->setinepoll();
    }
}
//从事件循环删除channel
void Epoll::removechannel(Channel* ch)
{
    if(ch->ineopll()){
        // std::cout<<"ch remove"<<std::endl;
        if(epoll_ctl(epollfd_,EPOLL_CTL_DEL,ch->fd(),0)==-1){
            perror("epoll_ctl failed\n");
            exit(-1);
        }
    }
}
std::vector<Channel*> Epoll::loop(int timeout){
    std::vector<Channel*> channels;
    bzero(events_,sizeof(events_));
    int infds=epoll_wait(epollfd_,events_,MaxEvents,timeout);
    // 返回失败。
    if (infds < 0)
    {
        //EBADF:epollfd无效操作符
        //EFAULT:参数events指向的内存不可写
        //EINVAL:epollfd不是epoll的文件描述符，或者maxevents小于0
        //EINTR:被信号中断
        perror("epoll_wait() failed"); exit(-1);
    }

    // 超时。
    if (infds == 0)
    {
        //超时表示epoll很空闲，回调tcpserver::epolltimeout()
        //printf("epoll_wait() timeout.\n"); 
        return channels;;
    }

    // 如果infds>0，表示有事件发生的fd的数量。
    for (int i=0;i<infds;i++)       // 遍历epoll返回的数组evs。
    {
        Channel *ch=(Channel*)events_[i].data.ptr;
        ch->setrevents(events_[i].events);
        channels.emplace_back(ch);
    }
    return channels;
}