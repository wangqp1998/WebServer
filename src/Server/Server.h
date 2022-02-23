#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "../Socket/Socket.h"
#include "../Epoller/Epoller.h"
#include <memory>  //unique_ptr
#include <unordered_map>
#include "../Http/HttpServer.h"
#include "../Log/Log.h"
#include "../pool/threadpool.h"

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
    void DealRead(HttpServer* client);
    void OnRead(HttpServer* client);

    void DealWrite(HttpServer* client);
    void OnWrite(HttpServer* client);
    
    void OnProsse(HttpServer* client);

    void CloseConn(HttpServer* client);
private:
    std::unique_ptr<Epoller> myepoller;
    std::unique_ptr<Socket> mysocket;
    std::unordered_map<int,HttpServer> user;
    std::unique_ptr<ThreadPool> mythreadpool;

};
}

#endif