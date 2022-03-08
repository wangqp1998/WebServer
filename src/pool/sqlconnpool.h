#ifndef SQLCONNPOLL_H
#define SQLCONNPOLL_H

#include <mysql/mysql.h>
#include <assert.h>
#include <queue>
#include <semaphore.h>
#include <mutex>

//#include "/usr/include/mysql/mysql.h"
#include "../Log/Log.h"
namespace WebServer
{
class SqlConnPool 
{
public:

    static SqlConnPool *Instance() { static SqlConnPool connPool; return &connPool;}
    
    void Init(const char* host, int port,
              const char* user,const char* pwd, 
              const char* dbName, int connSize);

    MYSQL *GetConn();
    void FreeConn(MYSQL * conn);

     void ClosePool();

    int GetFreeConnCount(){
    std::lock_guard<std::mutex> locker(mtx);
    return connQue.size();}
private:
    SqlConnPool():userCount(0) {}
    ~SqlConnPool(){ClosePool();}

    int userCount;
    int MAX_CONN;    //最大连接数

    std::queue<MYSQL *> connQue;
    std::mutex mtx;
    sem_t semId;
};
}

#endif