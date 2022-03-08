#include "Server.h"
#include <iostream>

WebServer::Server::Server(int port)
    :myepoller(new Epoller),mysocket(new Socket),
    mythreadpool(new ThreadPool(8)),mytimer(new Timer),
    timeoutMS(60000),myport(port)
{
    /*初始化SQL*/
    SqlConnPool::Instance()->Init("localhost",330,"root","999923","WebServerdb",12);  
    /*HttpServer静态变量赋值  mysrcDir为资源地址 userCount为当前连接的用户个数*/
    HttpServer::mysrcDir = "../resources";
    HttpServer::userCount = 0;
    /*初始化Epoll模式*/
    InitEventMode(0);
    /*初始化Socker*/
    InitSocket(port);
    /*初始化Log*/
    Log::Instance()->init(0, "./log", ".log", 1024);
    LOG_INFO("========== Server init ==========");
    
}

WebServer::Server::~Server()
{
    close(myport);
    SqlConnPool::Instance()->ClosePool();
}

void WebServer::Server::InitEventMode(int trigMode) {
    listenEvent = EPOLLRDHUP;
    connEvent = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        connEvent |= EPOLLET;
        break;
    case 2:
        listenEvent |= EPOLLET;
        break;
    case 3:
        listenEvent |= EPOLLET;
        connEvent |= EPOLLET;
        break;
    default:
        listenEvent |= EPOLLET;
        connEvent |= EPOLLET;
        break;
    }
    HttpServer::isET = (connEvent & EPOLLET);
}

void WebServer::Server::DealListen()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int fd;
    do {
        fd = mysocket->Accept((struct sockaddr *)& addr,len);
        if(fd <= 0) { return;}
        /*
        else if(HttpServer::userCount >= MAX_FD) {
            SendError(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }*/
        user[fd].Init(fd,addr);
        if(timeoutMS > 0) {
        mytimer->add(fd, timeoutMS, std::bind(&WebServer::Server::CloseConn, this, &user[fd]));
        }
        myepoller->AddFd(fd,EPOLLIN | connEvent);
    } while(listenEvent & EPOLLET);
   
    LOG_INFO("DealListen:%d",fd);
}

void WebServer::Server::DealRead(HttpServer* client)
{
    assert(client);
    ExtentTime(client);
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
    ExtentTime(client);
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
            myepoller->ModFd(client->GetFd(), EPOLLOUT|connEvent);
            return;
        }
    }
    //mytimer->dowork(client->GetFd());
    CloseConn(client);
}

void WebServer::Server::ExtentTime(HttpServer* client) {
    assert(client);
    if(timeoutMS > 0) {  mytimer->adjust(client->GetFd(),timeoutMS); }
}

void WebServer::Server::OnProsse(HttpServer* client)
{
    if(client->process())
    {
        myepoller->ModFd(client->GetFd(),EPOLLOUT|connEvent);
    }
    else
    {
        myepoller->ModFd(client->GetFd(),EPOLLIN|connEvent);
    }
}

void WebServer::Server::start()
{
    int timeMs = -1;  //无事件 阻塞
    
    while(1)
    {
        if(timeoutMS>0)
        {
            timeMs = mytimer->GetNextTick();
        }
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

    myepoller->AddFd(mysocket->Getlistedfd(),listenEvent|EPOLLIN);
}

void WebServer::Server::CloseConn(HttpServer* client)
{
    assert(client);
    LOG_INFO("CloseConn%d",client->GetFd());
    myepoller->DelFd(client->GetFd());
    client->Close();
}