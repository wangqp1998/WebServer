#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "../Socket/Socket.h"
#include "../Epoller/Epoller.h"
#include <memory>  //unique_ptr

namespace WebServer
{
class Server
{
public:
    Server(int port);
    ~Server();

    void InitSocket(int port);

    void DealListen();
    void start();
private:
    std::unique_ptr<Epoller> myepoller;
    std::unique_ptr<Socket> mysocket;
};
}

#endif