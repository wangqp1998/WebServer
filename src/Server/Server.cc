#include "Server.h"
#include <iostream>

WebServer::Server::Server(int port)
    :myepoller(new Epoller),mysocket(new Socket),mythreadpool(new ThreadPool(8))
{
    InitSocket(port);
    HttpServer::mysrcDir = "../resources";
    HttpServer::userCount = 0;

    Log::Instance()->init(0, "./log", ".log", 1024);
    LOG_INFO("========== Server init ==========");

}

WebServer::Server::~Server()
{
    
}
void WebServer::Server::DealListen()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int fd = mysocket->Accept((struct sockaddr *)& addr,len);
    user[fd].Init(fd,addr);
    myepoller->AddFd(fd,EPOLLIN|EPOLLONESHOT|EPOLLRDHUP);
    LOG_INFO("DealListen:%d",fd);
}

void WebServer::Server::DealRead(HttpServer* client)
{
    assert(client);
    LOG_INFO("DealRead:%d",client->GetFd());
    mythreadpool->AddTask(std::bind(&WebServer::Server::OnRead,this,client));
}
void WebServer::Server::OnRead(HttpServer* client)
{
    assert(client);
    int ret = -1;
    int readErrno=0;
    ret = client->read(&readErrno);
    OnProsse(client);   
}

void WebServer::Server::DealWrite(HttpServer* client)
{
    assert(client);
    LOG_INFO("DealWrite:%d",client->GetFd());
    mythreadpool->AddTask(std::bind(&WebServer::Server::OnWrite,this,client));
}

void WebServer::Server::OnWrite(HttpServer* client)
{
    int WriteError = 0;
    int ret = client->write(&WriteError);
    if(client->ToWriteBytes() == 0)
    {
        if(client->IsKeepAlive()) {
            OnProsse(client);
            return;
        }
    }
    else if(ret < 0) 
    {
        if(WriteError == EAGAIN) {
            /* 继续传输 */
            myepoller->ModFd(client->GetFd(), EPOLLOUT|EPOLLONESHOT|EPOLLRDHUP);
            return;
        }
    }
    CloseConn(client);
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
            else if(event & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                CloseConn(&user[fd]);
            }
            else if(event& EPOLLIN)
            {
                DealRead(&user[fd]);
            }
            else if(event& EPOLLOUT)
            {
                DealWrite(&user[fd]);
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

void WebServer::Server::CloseConn(HttpServer* client)
{
    assert(client);
    LOG_INFO("CloseConn%d",client->GetFd());
    myepoller->DelFd(client->GetFd());
    client->Close();
}