/*
 * 程序名：tcpepoll.cpp，此程序用于演示采用epoll模型实现网络通讯的服务端。
 * 作者：hyh
*/
#include "comFun.h"
#include "EchoServer.h"
#include <signal.h>
// #include <sys/signalfd.h>//信号fd
using namespace std;
EchoServer *echoserver;
void Stop(int sig){
    printf("sig:%d\nechoserver终止\n",sig);
    delete echoserver;
    cout<<"delete echoserver"<<endl;
    exit(0);
}
int main(int argc,char *argv[])
{
    // signal(SIGALRM,fun);
    // alarm(5);
/*
    //定时器
    int tfd=timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK|TFD_CLOEXEC);
    struct itimerspec timeout;
    memset(&timeout,0,sizeof(struct itimerspec));
    timeout.it_value.tv_sec=5;
    timeout.it_interval.tv_nsec=0;
    timerfd_settime(tfd,0,&timeout,0);
    struct epoll_event ev;
    ev.data.fd=tfd;
    ev.events=EPOLLIN;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,tfd,&ev);
    //信号加入epoll
    sigset_t sigset;//创建信号集
    sigemptyset(&sigset);//清空信号集
    sigaddset(&sigset,SIGINT);//添加信号
    sigaddset(&sigset,SIGALRM);//添加信号
    sigprocmask(SIG_BLOCK,&sigset,0);//屏蔽信号
    int sigfd=signalfd(-1,&sigset,0);//创建sigfd
    ev.data.fd=sigfd;
    ev.events=EPOLLIN;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,sigfd,&ev);
*/
    signal(SIGTERM,Stop);//15,kill
    signal(SIGINT,Stop);//2,ctrl+c

    echoserver=new EchoServer(IP,PORT,3,5);
    echoserver->start();
    return 0;
}