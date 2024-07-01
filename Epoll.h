#pragma once
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>
#include "Channel.h"
class Channel;
class Epoll{
private:
    static const int MaxEvents=100;
    int epollfd_=-1;
    epoll_event events_[MaxEvents];
public:
    Epoll();
    ~Epoll();

    //void addfd(int fd,uint32_t op);//将fd添加到红黑树上
    std::vector<Channel*> loop(int timeout=-1);//运行epoll_wait事件循环，易发生的事件用vector返回
    void updatechannel(Channel* ch);
    void removechannel(Channel* ch);//从事件循环删除channel
};