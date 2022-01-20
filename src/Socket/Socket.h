#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

namespace WebServer
{
class Socket
{
public:
    Socket():listenfd(socket(AF_INET, SOCK_STREAM, 0))
       {
           if(listenfd < 0)
            {
                perror("socket error");
                exit(1);
            }        
       }
    ~Socket(){close(listenfd);}

    void SetElegantClose();
    void SetIOMult();
    void Bind(int port);
    void Listen();
    int Accept();

    int Getlistedfd(){return listenfd;}
    Socket(const Socket&) = delete;
    void operator=(const Socket&) = delete;
private:
    int listenfd;
};
}

#endif