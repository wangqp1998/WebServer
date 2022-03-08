#include "src/Server/Server.h"
#include "src/Log/Log.h"
#include "src/pool/sqlconnpool.h"
/*void test_Log()
{
    
    WebServer::Log::Instance()->init(1, "./log", ".log", 1024);       
    LOG_DEBUG("Log_debug");
    LOG_INFO("Log_info");
    LOG_WARN("Log_warn") ;
    LOG_ERROR("Log_error");
    
}
*/
int main()
{
    WebServer::Server myserver(9600);
    myserver.start();
    //WebServer::SqlConnPool::Instance()->Init("localhost",330,"root","999923","WebServerdb",12);
}