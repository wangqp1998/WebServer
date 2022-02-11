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
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int fd = mysocket->Accept((struct sockaddr *)& addr,len);
    user[fd].Init(fd,addr);
    myepoller->AddFd(fd,EPOLLIN|EPOLLONESHOT|EPOLLRDHUP);
}

void WebServer::Server::DealRead(HttpServer* client)
{
    assert(client);
    std::cout<<"开始读"<<std::endl;
    OnRead(client);
}
void WebServer::Server::OnRead(HttpServer* client)
{
    assert(client);
    int ret = -1;
    int readErrno=0;
    std::cout<<"开始读入buffer"<<std::endl;
    ret = client->read(&readErrno);
    std::cout<<"读入buffer完成"<<std::endl;
    
    OnProsse(client);   
}

void WebServer::Server::OnProsse(HttpServer* client)
{
    if(client->process())
    {
        myepoller->ModFd(client->GetFd(),EPOLLOUT|EPOLLONESHOT|EPOLLRDHUP);
    }
    else
    {
        myepoller->ModFd(client->GetFd(),EPOLLIN|EPOLLONESHOT|EPOLLRDHUP);
    }
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
               std::cout<<"DealListen"<<std::endl;
                DealListen();
                 
            }
            else if(event & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                
            }
            else if(event& EPOLLIN)
            {
                std::cout<<"DealRead"<<std::endl;
                DealRead(&user[fd]);

            }
            else if(event& EPOLLOUT)
            {

            }
            else 
            {
                
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