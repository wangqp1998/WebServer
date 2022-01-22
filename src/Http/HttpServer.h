#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "../Buffer/Buffer.h"
#include <arpa/inet.h>

namespace WebServer
{
class HttpServer
{
public:
    HttpServer();
    ~HttpServer();

    void Init(int fd,const sockaddr_in& addr);
    
    
private:

    Buffer* inputBuffer;
    Buffer* outputBuffer;

};

}


#endif