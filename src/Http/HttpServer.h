#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "../Buffer/Buffer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "../Log/Log.h"
#include <arpa/inet.h>
#include <memory>
#include <atomic>
#include <errno.h>

namespace WebServer
{
class HttpServer
{
public:
    HttpServer();
    ~HttpServer();

    void Init(int fd,const sockaddr_in& addr);

    ssize_t read(int* saveErrno);
    ssize_t write(int* saveErrno);
    bool process();

    int GetFd() {return Httpfd;}
    const char* GetIP() const {return inet_ntoa(Httpaddr.sin_addr);}
    int GetPort() const{ return Httpaddr.sin_port;}

    int ToWriteBytes() {return iov[0].iov_len+iov[1].iov_len;} 
    bool IsKeepAlive() {return HttpRequest_ptr->IsKeepAlive();}
    

    void Close();
public:    
    static std::atomic<int> userCount;
    static const char* mysrcDir;
    static bool isET;
private:

    std::unique_ptr<HttpRequest> HttpRequest_ptr;
    std::unique_ptr<HttpResponse> HttpResponse_ptr;
    int Httpfd;
    sockaddr_in Httpaddr;
    Buffer inputBuffer;
    Buffer outputBuffer;

    bool IsClose;
    int iovCnt;
    struct iovec iov[2];   
};



}


#endif