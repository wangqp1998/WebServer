#include "Server.h"
#include <iostream>

WebServer::Server::Server(int port)
    :myepoller(new Epoller),mysocket(new Socket)
{
    InitSocket(port);
}

WebServer::Server::~Server()
{
    
}
void WebServer::Server::DealListen()
{
    std::cout<<"新的连接"<<std::endl;
    myepoller->AddFd(mysocket->Accept(),EPOLLIN|EPOLLONESHOT|EPOLLRDHUP);
}

void WebServer::Server::start()
{
    int timeMs = -1;  //无事件 阻塞

    std::cout<<"====Server start===="<<std::endl;

    while(1)
    {
        int eventCnt = myepoller->Wait(timeMs);
        for(int i=0;i<eventCnt;i++)
        {
            int fd = myepoller->GetEventFd(i);
            uint32_t event=myepoller->GetEvents(i);
            if(fd==mysocket->Getlistedfd())
            {
                DealListen();
            }
        }
    } 
}

void WebServer::Server::InitSocket(int port)
{
    //设置优雅关闭
    mysocket->SetElegantClose();   
    //设置IO复用
    mysocket->SetIOMult();
    
    mysocket->Bind(port);

    mysocket->Listen();

    myepoller->AddFd(mysocket->Getlistedfd(),EPOLLRDHUP|EPOLLIN);
}