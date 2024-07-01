#include "EventLoop.h"
int createtimerfd(int sec=30){
    int tfd=timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK|TFD_CLOEXEC);
    struct itimerspec timeout;
    memset(&timeout,0,sizeof(struct itimerspec));
    timeout.it_value.tv_sec=sec;
    timeout.it_interval.tv_nsec=0;
    timerfd_settime(tfd,0,&timeout,0);
    return tfd;   
}
EventLoop::EventLoop(bool mainloop,int timeval,int timeout)
:mainloop_(mainloop),timeval_(timeval),timeout_(timeout)
,ep_(new Epoll),wakeupfd_(eventfd(0,EFD_NONBLOCK))
,wakeupch_(new Channel(this,wakeupfd_))
,timerfd_(createtimerfd(timeval_))
,timerch_(new Channel(this,timerfd_))
,stop_(false)
{
    wakeupch_->setreadcallback(std::bind(&EventLoop::handlewakeup,this));
    wakeupch_->enablereading();

    timerch_->setreadcallback(std::bind(&EventLoop::handletimer,this));
    timerch_->enablereading();
}
EventLoop::~EventLoop()
{
    // delete ep_;
    stop();
}

void EventLoop::run(){
    threadid_=syscall(SYS_gettid);
    while (!stop_)        // 事件循环。
    {
        std::vector<Channel*> channels=ep_->loop(10*1000);       // 等待监视的fd有事件发生。
        if(channels.empty()){// 如果channels为空，表示超时，回调TcpServer::epolltimeout()。
            epolltimeoutCB_(this);
        }
        for (auto& ch:channels)       // 遍历epoll返回的数组evs。
        {
            ch->handleevent();
        }
    }
}
// Epoll* EventLoop::ep(){
//     return ep_;
// }
// 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件
void EventLoop::updatechannel(Channel* ch)
{
    ep_->updatechannel(ch);
}

void EventLoop::setEpollTimeoutCB(std::function<void(EventLoop*)> fn)
{
    epolltimeoutCB_=fn;
}
//从事件循环删除channel
void EventLoop::removechannel(Channel* ch)
{
    ep_->removechannel(ch);
}
//判断当前线程是否为事件循环线程
bool EventLoop::isInLoopthread()
{
    return threadid_==syscall(SYS_gettid);
}

void EventLoop::queueinloop(std::function<void()> fn)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        taskqueue_.push(fn);
    }

    //唤醒事件循环
    wakeup();
}
//用eventfd唤醒
void EventLoop::wakeup()
{
    uint64_t val=1;
    write(wakeupfd_,&val,sizeof(val));
}
//事件循环被唤醒执行的函数
void EventLoop::handlewakeup()
{
    // printf("handlewakeup thread id is %ld\n",syscall(SYS_gettid));
    uint64_t val;
    read(wakeupfd_,&val,sizeof(val));//从eventfd中读取数据，若不读取，水平触发会一直触发

    std::function<void()> fn;
    std::lock_guard<std::mutex> lock(mutex_);
    //执行全部发送任务
    while(taskqueue_.size()>0){
        fn=std::move(taskqueue_.front());
        taskqueue_.pop();
        fn();
    }
}
//闹钟响起执行函数
void EventLoop::handletimer()
{
    //重新计时
    struct itimerspec timeout;
    memset(&timeout,0,sizeof(struct itimerspec));
    timeout.it_value.tv_sec=timeval_;
    timeout.it_interval.tv_nsec=0;
    timerfd_settime(timerfd_,0,&timeout,0);
    if(mainloop_){

        // printf("mainloop\n");
    }
    else{
        // printf("subloop\n");
        // printf("EventLoop::handletimer thread is %ld. fd ",syscall(SYS_gettid));
        time_t now=time(0);
        //只能用显式for循环删除
        for(auto it=conns_.begin();it!=conns_.end();){
            // printf("%d ",it->first);
            if(it->second->timeout(now,timeout_)){
                //下面两句不能交换顺序，否则it变
                removeconnCB_(it->first);
                {
                    std::lock_guard<std::mutex> lock(mmutex_);
                    it=conns_.erase(it);
                }
                
            }else{
                ++it;
            }
        }
        // printf("\n");
    }
}

void EventLoop::newconnection(spConnection conn)
{
    std::lock_guard<std::mutex> lock(mmutex_);
    conns_[conn->fd()]=conn;
}

void EventLoop::setremoveconnCB(std::function<void(int)> fn)
{
    removeconnCB_=fn;
}

void EventLoop::stop()
{
    stop_=true;
    //必须立即唤醒事件循环，才会直接停止,否则必须等到闹钟响起或者epoll_wait超时才行
    wakeup();
}