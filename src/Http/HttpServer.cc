#include "HttpServer.h"
#include <iostream>
std::atomic<int> WebServer::HttpServer::userCount;

WebServer::HttpServer::HttpServer()
    :Httpfd(-1),Httpaddr{0},HttpRequest_ptr(new HttpRequest)
{
}

WebServer::HttpServer::~HttpServer()
{

}

ssize_t WebServer::HttpServer::read(int* saveErrno)
{
    ssize_t len = -1;
   
        len=inputBuffer.readFd(Httpfd,saveErrno);
        //if(len <= 0)
           // break;
   
    return len;
}

void WebServer::HttpServer::Init(int fd,const sockaddr_in& addr)
{
    Httpfd = fd;
    Httpaddr = addr;
    inputBuffer.RetrieveAll();
    outputBuffer.RetrieveAll();
}

bool WebServer::HttpServer::process()
{
    std::cout<<"Prosse"<<std::endl;
    HttpRequest_ptr->init();
    std::cout<<"初始化完成"<<std::endl;
    if(inputBuffer.readableBytes() <= 0)  /*无可读内容 */   
    {
        return false;
    }
    else if(HttpRequest_ptr->pares(inputBuffer))
    {
        std::cout<<"连接成功"<<std::endl;
    }
    else
    {
        printf("连接失败");
    }
    return true;
}
