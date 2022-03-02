#include "Epoller.h"


bool WebServer::Epoller::AddFd(int fd, uint32_t events)
{
    if(fd < 0) return false;
    epoll_event ev={0};     //一个epoll事件
    ev.data.fd=fd;
    ev.events=events;
    return 0==epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);  //添加
}

bool WebServer::Epoller::ModFd(int fd, uint32_t events)
{
    if(fd < 0) return false;
    epoll_event ev={0};     //一个epoll事件
    ev.data.fd=fd;
    ev.events=events;
    return 0==epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);   
}

bool WebServer::Epoller::DelFd(int fd)
{
    if(fd < 0) return false;
    epoll_event ev={0};     //一个epoll事件
    ev.data.fd=fd;
    return 0==epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev);   //移除
}

int WebServer::Epoller::Wait(int timeoutMs)
{
    return epoll_wait(epollfd, &events[0], static_cast<int>(events.size()), timeoutMs);
}

int WebServer::Epoller::GetEventFd(size_t i) const
{
    assert(i < events.size() && i >= 0);
    return events[i].data.fd;
}

uint32_t WebServer::Epoller::GetEvents(size_t i) const
{
    assert(i < events.size() && i >= 0);
    return events[i].events;
}