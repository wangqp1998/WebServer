#include "Server.h"
#include <iostream>

WebServer::Server::Server(int port)
    :myepoller(new Epoller),mysocket(new Socket)
{
    InitSocket(port);
    HttpServer::mysrcDir = "../resources";
    HttpServer::userCount = 0;
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
    std::cout << "连接:" << fd <<std::endl;
}

void WebServer::Server::DealRead(HttpServer* client)
{
    assert(client);
    std::cout << "读数据:" << client->GetFd() <<std::endl;
    OnRead(client);
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
    int WriteError = 0;
    std::cout << "写数据:" << client->GetFd() <<std::endl;
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
                std::cout<<"DealWrite"<<std::endl;
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
    std::cout <<"关闭:"<< client->GetFd() << std::endl;
    myepoller->DelFd(client->GetFd());
    client->Close();
}