#pragma once
#include <mutex>
#include <unistd.h>
#include <thread>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <vector>
#include <queue>
#include <sys/syscall.h>
#include <iostream>
class ThreadPool{
private:
    std::vector<std::thread> threads_;//线程池中的线程数
    std::queue<std::function<void()>> taskqueue_;//任务队列
    std::mutex mutex_;//任务队列互斥锁
    std::condition_variable condition_;//任务队列同步条件变量
    std::atomic_bool stop_;//在析构函数中，把stop_设置为true，全部线程结束

    std::string threadtype_;//线程种类：WORK、IO

public:
    //创建threadnum个线程   
    ThreadPool(size_t threadnum,const std::string &threadtype);
    //析构函数中停止线程
    ~ThreadPool();
    //把任务添加到任务队列
    void addtask(std::function<void()> task);
    size_t size();
    void stop();
};