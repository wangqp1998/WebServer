#include "src/Server/Server.h"

int main()
{
    WebServer::Server myserver(9600);
    myserver.start();
}