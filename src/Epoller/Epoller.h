#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h> 
#include <fcntl.h>  
#include <unistd.h> 
#include <assert.h> 
#include <vector>
#include <errno.h>

namespace WebServer
{

class Epoller
{
public:
    Epoller(const int maxEvent = 1024):epollfd(epoll_create(512)),events(maxEvent)
        {assert(epollfd >= 0 && events.size() > 0);}

    ~Epoller(){ close(epollfd);}

    bool AddFd(int fd, uint32_t events);

    bool ModFd(int fd, uint32_t events);

    bool DelFd(int fd);

    int Wait(int timeoutMs = -1);

    int GetEventFd(size_t i) const;

    uint32_t GetEvents(size_t i) const;

    //
    Epoller(const Epoller&) = delete;
    void operator=(const Epoller&) = delete;

private:
    int epollfd;

    std::vector<struct epoll_event> events;
};
    
}//WebServer


#endif