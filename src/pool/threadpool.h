#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <assert.h>

namespace WebServer
{
class ThreadPool
{
public:
    explicit ThreadPool(size_t threadCount=8)    //默认8个线程
    :mypool(std::make_shared<Pool>())
    {
        assert(threadCount>0);
        for(size_t i=0;i<threadCount;i++)
        {
            std::thread([pool=mypool]{
                std::unique_lock<std::mutex> locker(pool->mtx);
                while(true)
                {
                    if(!pool->tasks.empty())
                    {
                        auto task = std::move(pool->tasks.front());
                        pool->tasks.pop();
                        locker.unlock();
                        task();
                        locker.lock();
                    }
                    else if(pool->isClosed) break;
                    else pool->cond.wait(locker);
                }
            }).detach();
        }
    }
    ThreadPool() = default;

    ThreadPool(ThreadPool&&) = default;
    
    ~ThreadPool() {
        if(static_cast<bool>(mypool)) {
            {
                std::lock_guard<std::mutex> locker(mypool->mtx);
                mypool->isClosed = true;
            }
            mypool->cond.notify_all();
        }
    }

    template<class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(mypool->mtx);
            mypool->tasks.emplace(std::forward<F>(task));
        }
        mypool->cond.notify_one();
    }

private:
    struct Pool
    {
        std::mutex mtx;
        std::condition_variable cond;
        bool isClosed;
        std::queue<std::function<void()>> tasks;
    };
    
    std::shared_ptr<Pool> mypool;
};

}
#endif