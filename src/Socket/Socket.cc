#include "Socket.h"

/* 优雅关闭: 直到所剩数据发送完毕或超时 */
void WebServer::Socket::SetElegantClose()
{
    struct linger optLinger = { 0 };
    optLinger.l_onoff = 1;
    optLinger.l_linger = 1;
    int ret = setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        close(listenfd);
    }
}

void WebServer::Socket::SetIOMult()
{
     int optval = 1;
    /* 端口复用 */
    /* 只有最后一个套接字会正常接收数据。 */
    int ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        perror("SetIOMult error");
        //LOG_ERROR("set socket setsockopt error !");
        close(listenfd);
        exit(1);
    }
}

void WebServer::Socket::Bind(int port)
{
    struct sockaddr_in addr;
    if(port > 65535 || port < 1024) {
        //LOG_ERROR("Port:%d error!",  port);
        //return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    int ret = bind(listenfd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        //LOG_ERROR("Bind Port:%d error!", port_);
        close(listenfd);
        perror("bind error");
        exit(1); 
    }
}
void WebServer::Socket::Listen()
{
    int ret = listen(listenfd, 6);
    if(ret < 0) {
        perror("listen error");
        //LOG_ERROR("Listen port:%d error!", port_);
        close(listenfd);
        exit(1);
    }
}

int WebServer::Socket::Accept(struct sockaddr * addr, socklen_t len)
{
    int fd = accept(listenfd, addr, &len);
    if(fd <= 0){ return -1;}
    return fd;
}