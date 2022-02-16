#include "HttpServer.h"
#include <iostream>
std::atomic<int> WebServer::HttpServer::userCount;
const char* WebServer::HttpServer::mysrcDir;

WebServer::HttpServer::HttpServer()
    :Httpfd(-1),Httpaddr{0},HttpRequest_ptr(new HttpRequest),HttpResponse_ptr(new HttpResponse),IsClose(true)
{
}

WebServer::HttpServer::~HttpServer()
{
    Close();
}

ssize_t WebServer::HttpServer::read(int* saveErrno)
{
    ssize_t len = -1;
   
        len=inputBuffer.readFd(Httpfd,saveErrno);
        //if(len <= 0)
           // break;
   
    return len;
}

ssize_t WebServer::HttpServer::write(int* saveErrno)
{
    ssize_t len = -1;
    do
    {
        len = writev(Httpfd,iov,iovCnt);
        writev(STDOUT_FILENO,iov,iovCnt);
        if(len <= 0)
        {
            *saveErrno = errno;
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
    } while (0);
 
    return len;
}

void WebServer::HttpServer::Init(int fd,const sockaddr_in& addr)
{
    Httpfd = fd;
    Httpaddr = addr;
    inputBuffer.RetrieveAll();
    outputBuffer.RetrieveAll();
    IsClose = true;
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
        std::cout<<"文件!!"<<std::endl;
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
    }
}