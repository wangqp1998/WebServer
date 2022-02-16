#include "../src/Socket/Socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/uio.h>
#include <iostream>

#define SERV_PORT 9600

int main()
{
    int cfd;
    int conter = 10;
    char buf[BUFSIZ];
    std::string str0="GET / HTTP/1.1 ";
    str0 += "/r/n/r/n";
    str0 +="Host: localhost:9600" ;
    str0 += "/r/n";
    str0 += "Connection: keep-alive";
    str0 += "/r/n";

    char* str = (char*)str0.data();
    struct iovec vec[2];

    struct sockaddr_in serv_addr;          //服务器地址结构

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr); 
   
    cfd = socket(AF_INET, SOCK_STREAM, 0);
    int ret = connect(cfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    vec[0].iov_base =str;
    vec[0].iov_len =strlen(str) + 1;

    writev(cfd, vec, 1);
    
    while(1)
    {
        ret = read(cfd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, ret);
    }
    close(cfd);

	return 0;
}
