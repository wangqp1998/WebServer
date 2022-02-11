#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "../Buffer/Buffer.h"
#include "HttpRequest.h"
#include <arpa/inet.h>
#include <memory>
#include <atomic>

namespace WebServer
{
class HttpServer
{
public:
    HttpServer();
    ~HttpServer();

    void Init(int fd,const sockaddr_in& addr);

    ssize_t read(int* saveErrno);
    bool process();

    int GetFd() {return Httpfd;}
public:    
    static std::atomic<int> userCount;
    
private:

    std::unique_ptr<HttpRequest> HttpRequest_ptr;
    int Httpfd;
    sockaddr_in Httpaddr;
    Buffer inputBuffer;
    Buffer outputBuffer;

    
};



}


#endif