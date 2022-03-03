#include "timer.h"

void WebServer::Timer::add(int id,int time_out,const TimeoutCallBack& cb)
{
    assert(id >= 0);
    size_t i;
    if(ref.count(id)==0)
    {
        i=heap.size();
        ref[id] = i;    //记录id在head里面的位置
        heap.push_back({id,Clock::now()+MS(time_out),cb});
        siftup(i);  //调整当前节点位置

    }
    else
    {
        i=ref[id];   //找到当前id的位置
        heap[i].expires=Clock::now()+MS(time_out);     //更新时间
        heap[i].cb=cb;
        if(!siftdown(i,heap.size()))    //向下调整位置 
            siftup(i);  //如果不需要向下调整，就向上调整位置
    }
    LOG_INFO("Timer add : id=%d,count:%d",id,heap.size());
}
void WebServer::Timer::siftup(size_t i)
{
    assert(i>=0&&i<heap.size());   //位置i在heap里
    size_t j=(i-1)/2;   //找到其父节点
    while(j>0)
    {
        if(heap[j] < heap[i])    //父节点<字节点 不需要调整
            break;
        SwapNode(i,j);
        i=j;
        j=(i-1)/2;    //基于和父节点比较
    }

}

void WebServer::Timer::SwapNode(size_t i,size_t j)
{
    assert(i>=0&&i<heap.size());
    assert(j>=0&&j<heap.size());   //两个节点均在heap中
    std::swap(heap[i],heap[j]);   //交换
    ref[heap[i].id] = i;
    ref[heap[j].id] = j;   //更新位置信息
}

bool WebServer::Timer::siftdown(size_t index,size_t n)
{
    assert(index>=0&&index<heap.size());
    assert(n>=0&&n<=heap.size());   //两个节点均在heap中
    size_t i=index;
    size_t j=i*2+1;   //右子节点
    while (j<n)
    {
        if(j+1<n&&heap[j+1]<heap[j]) j++;   //选出节点中最小的
        if(heap[i]<heap[j])  break;  //当前节点比子节点大 ,位置成立
        SwapNode(i,j); 
        i=j;
        j=i*2+1;
    }
    return i>index;    //发送变化返回1
    
}

void WebServer::Timer::del(size_t index)     
{
    assert(!heap.empty() && index>=0 && index < heap.size());
    size_t i=index;
    size_t n=heap.size()-1;
    assert(i<=n);
    if(i<n)
    {
        SwapNode(i,n);   //把当前位置换到最后
        ref.erase(heap.back().id);
        heap.pop_back();
        if(!siftdown(i,n))
            siftup(i);
    }
    else
    {
        ref.erase(heap.back().id);
        heap.pop_back();
    }
    /*
    ref.erase(heap.back().id);
    heap.pop_back();*/
}

void WebServer::Timer::dowork(int id)
{
    //assert(id>=0&&id<heap.size());
    if(heap.empty()||ref.count(id)==0)  return;
    size_t i = ref[id];
    TimerNode node = heap[i];
    node.cb();     //调用回调函数
    del(i);   //删除节点
    LOG_INFO("Timer del: id=%d,count:%d",id,heap.size());
}

void WebServer::Timer::tick() {
    /* 清除超时结点 */
    if(heap.empty()) {
        return;
    }
    while(!heap.empty()) {
        TimerNode node = heap.front();
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) { 
            break; 
        }
        node.cb();
        pop();
    }
}

int WebServer::Timer::GetNextTick() {
    tick();
    size_t res = -1;
    if(!heap.empty()) {
        res = std::chrono::duration_cast<MS>(heap.front().expires - Clock::now()).count();
        if(res < 0) { res = 0; }
    }
    return res;
}

void WebServer::Timer::adjust(int id, int timeout) {
    /* 调整指定id的结点 */
    assert(!heap.empty() && ref.count(id) > 0);
    heap[ref[id]].expires = Clock::now() + MS(timeout);
    siftdown(ref[id], heap.size());
}