#include "HttpServer.h"
#include <iostream>

std::atomic<int> WebServer::HttpServer::userCount;
const char* WebServer::HttpServer::mysrcDir;
bool WebServer::HttpServer::isET;

WebServer::HttpServer::HttpServer()
    :Httpfd(-1),Httpaddr{0},HttpRequest_ptr(new HttpRequest),HttpResponse_ptr(new HttpResponse),IsClose(true)
{
}

WebServer::HttpServer::~HttpServer()
{
    Close();    //断开连接
}

ssize_t WebServer::HttpServer::read(int* saveErrno)   //读
{
    ssize_t len = -1;
    do
    {
        len=inputBuffer.readFd(Httpfd,saveErrno);
        if(len <= 0)
            break;
    }while(isET);
    
    return len;
}

ssize_t WebServer::HttpServer::write(int* saveErrno)  //写
{
    ssize_t len = -1;
    do
    {
        len = writev(Httpfd,iov,iovCnt);
        LOG_INFO("writev");
        if(len <= 0)
        {
            *saveErrno = errno;
            break;
        }
        if(iov[0].iov_len+iov[1].iov_len == 0)   //传输完成
        {
            break;
        }
        else if(static_cast<size_t>(len) > iov[0].iov_len)  /*?????*/
        {
            iov[1].iov_base = (uint8_t*) iov[1].iov_base + (len - iov[0].iov_len);
            iov[1].iov_len -= (len - iov[0].iov_len);
            if(iov[0].iov_len) {
                outputBuffer.RetrieveAll();
                iov[0].iov_len = 0;
            }
        }
        else
        {
            iov[0].iov_base = (uint8_t*)iov[0].iov_base + len; 
            iov[0].iov_len -= len; 
            outputBuffer.Retrieve(len);
        }
    } while (isET||ToWriteBytes() > 10240);
 
    return len;
}

void WebServer::HttpServer::Init(int fd,const sockaddr_in& addr)
{
    assert(fd>0);
    userCount++;   //用户加1
    Httpfd = fd;
    Httpaddr = addr;
    inputBuffer.RetrieveAll();   //清空input区
    outputBuffer.RetrieveAll();  //情况output区
    IsClose = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", Httpfd, GetIP(), GetPort(), (int)userCount);
}

bool WebServer::HttpServer::process()
{
    HttpRequest_ptr->init();
    if(inputBuffer.readableBytes() <= 0)  /*无可读内容 */   
    {
        return false;
    }
    else if(HttpRequest_ptr->pares(inputBuffer))
    {
        HttpResponse_ptr->Init(mysrcDir,HttpRequest_ptr->Getpath(),200,HttpRequest_ptr->IsKeepAlive());        
    }
    else
    {
        HttpResponse_ptr->Init(mysrcDir,HttpRequest_ptr->Getpath(),400,HttpRequest_ptr->IsKeepAlive());        
    }
    HttpResponse_ptr->MakeResponse(outputBuffer);

    /*响应头*/
    iov[0].iov_base = const_cast<char*>(outputBuffer.Peek());
    iov[0].iov_len = outputBuffer.readableBytes();
    iovCnt = 1;
    /*文件*/
    if(HttpResponse_ptr->GetFile()&&HttpResponse_ptr->GetFileSize())
    {
        iov[1].iov_base = HttpResponse_ptr->GetFile();
        iov[1].iov_len = HttpResponse_ptr->GetFileSize();
        iovCnt = 2;
    }
    return true;
}

void WebServer::HttpServer::Close()
{
    HttpResponse_ptr->UnmapFile();
    if(!IsClose)
    {
        IsClose = true;
        userCount -- ;
        close(Httpfd);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", Httpaddr, GetIP(), GetPort(), (int)userCount);
    }
}