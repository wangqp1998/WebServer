#ifndef TIMER_H
#define TIMER_H


#include<functional>
#include<time.h>
#include<chrono>
#include<vector>
#include<unordered_map>
#include<assert.h>
#include "../Log/Log.h"

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

namespace WebServer
{
struct TimerNode
{
    int id;
    TimeStamp expires;
    TimeoutCallBack cb;   //回调函数
    bool operator<(const TimerNode& t)   //重载<
    {
        return expires < t.expires;
    }

};


class Timer
{
public:
    Timer()
    {
        heap.reserve(64);
        LOG_INFO("=======Timer Start=======");
    };
    ~Timer(){ clear(); }

    void add(int id,int time_out,const TimeoutCallBack& cb);   //增加一个时间节点到堆中

    void clear(){ ref.clear();heap.clear(); }

    void pop(){ assert(!heap.empty()); del(0);}

    void dowork(int id);

    void adjust(int id,int timeout);

    void tick();
    
    int GetNextTick();

private:
    void del(size_t index);  //删除位置为i的节点

    void siftup(size_t i);      //向上调整节点位置

    void SwapNode(size_t i,size_t j);     //交换两个节点

    bool  siftdown(size_t index,size_t n);  //向下调整位置

    std::vector<TimerNode> heap;    //最小堆
    std::unordered_map<int,size_t> ref;    //id  位置

};
} // namespace Werserver



#endif