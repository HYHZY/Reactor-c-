#include "ThreadPool.h"


//创建threadnum个线程   
ThreadPool::ThreadPool(size_t threadnum,const std::string &threadtype)
:stop_(false),threadtype_(threadtype)
{
    for(size_t i=0;i<threadnum;++i){
        threads_.emplace_back([this]
        {
            printf("create %s thread(%ld).\n",threadtype_.c_str(),syscall(SYS_gettid));//显示线程号
            //每个线程都在执行这个while循环
            while(!stop_)
            {
                std::function<void()> task;
                //从任务队列拿一个任务开始执行，访问任务队列用互斥锁
                //锁作用域开始
                {
                    //lock: 必须是一个 std::unique_lock<std::mutex> 
                    //对象，通过该对象来管理与 std::condition_variable 
                    //相关联的 std::mutex 的锁定状态。
                    std::unique_lock<std::mutex> lock(this->mutex_);

                    //wait producer condition
                    //必须先获得 std::unique_lock<std::mutex> 的锁
                    this->condition_.wait(lock,[this]
                    {
                        //返回true，条件变量被触发，线程往下走
                        return ((this->stop_==true)||(this->taskqueue_.empty()==false));
                    });
                    //在线程池停止前，如果队列中还有任务，执行完在退出
                    //stop_==true和task队列不为空是唤醒线程的条件
                    if((this->stop_==true)&&(this->taskqueue_.empty()==true)) return;

                    //出队一个任务
                    task=std::move(this->taskqueue_.front());
                    this->taskqueue_.pop();
                }//锁作用域结束
                
                //执行任务
                if (task) {  // 检查 task 是否被初始化
                    // printf("%s(%ld) execute task.\n",threadtype_.c_str(),syscall(SYS_gettid));//显示线程号
                    task();
                } else {
                    // std::cout<<"task is not initialized"<<std::endl;
                    std::cerr << "Task is not initialized!" << std::endl;
                }
                // task();
            }
        });
    }
}
//析构函数中停止线程
ThreadPool::~ThreadPool()
{
    stop();
}
//把任务添加到任务队列
void ThreadPool::addtask(std::function<void()> task)
{
    {//锁作用域
        std::lock_guard<std::mutex> lock(mutex_);
        taskqueue_.push(task);
    }
    //唤醒一个线程
    condition_.notify_one();
}
size_t ThreadPool::size()
{
    return threads_.size();
}

void ThreadPool::stop()
{
    if(stop_){
        return;
    }
    stop_=true;//所有线程不在阻塞，条件变量唤醒
    condition_.notify_all();//唤醒全部线程
    //等待全部线程执行完毕退出
    for(std::thread &th:threads_){
        th.join();
    }
}
// void show(int no,std::string& name){
//     printf("I am %d %s\n",no,name.c_str());
// }
// void test(){
//     printf("I love you\n");
// }

// int main(){
//     ThreadPool threadpool(3);
//     std::string name="xixix";
//     threadpool.addtask(std::bind(show,8,name));
//     sleep(1);
//     threadpool.addtask(std::bind(test));
//     sleep(1);
//     threadpool.addtask(std::bind([]{printf("fesvsvfddfb\n");}));
//     sleep(1);
// }

//g++ -g -o threadpool ThreadPool.cpp -lpthread