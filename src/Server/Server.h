#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "../Socket/Socket.h"
#include "../Epoller/Epoller.h"
#include <memory>  //unique_ptr
#include <unordered_map>
#include "../Http/HttpServer.h"
#include "../Log/Log.h"
#include "../pool/threadpool.h"
#include "../timer/timer.h"
namespace WebServer
{
class Server
{
public:
    Server(int port);
    ~Server();

    void InitSocket(int port);
    void InitEventMode(int trigMode);

    void DealListen();
    void start();
    void DealRead(HttpServer* client);
    void OnRead(HttpServer* client);

    void DealWrite(HttpServer* client);
    void OnWrite(HttpServer* client);
    
    void OnProsse(HttpServer* client);

    void CloseConn(HttpServer* client);

    void ExtentTime(HttpServer* client);
private:

    uint32_t listenEvent;
    uint32_t connEvent;

    int timeoutMS;

    std::unique_ptr<Epoller> myepoller;
    std::unique_ptr<Socket> mysocket;
    std::unique_ptr<Timer> mytimer;
    std::unordered_map<int,HttpServer> user;
    std::unique_ptr<ThreadPool> mythreadpool;

};
}

#endif