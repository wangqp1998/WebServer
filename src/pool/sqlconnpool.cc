#include "sqlconnpool.h"

void WebServer::SqlConnPool::Init(const char* host, int port,
            const char* user,const char* pwd, const char* dbName,
            int connSize = 10) {
    assert(connSize > 0);
    for (int i = 0; i < connSize; i++) {
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);    //分配mysql新对象
        if (!sql) {
            LOG_ERROR("MySql init error!");
            assert(sql);
        }
        sql = mysql_real_connect(sql, host,
                                 user, pwd,
                                 dbName, port, nullptr, 0);  //连接数据库
        if (!sql) {
            LOG_ERROR("MySql Connect error!");
        }
        connQue.push(sql);   //进入队列
    }
    MAX_CONN = connSize;
    sem_init(&semId, 0, MAX_CONN);   //信号量,semId指向信号量结构的指针,0:线程共享,MAX_CONN 初始值
}

MYSQL* WebServer::SqlConnPool::GetConn() {
    MYSQL *sql = nullptr;
    if(connQue.empty()){    //当前队列为空
        LOG_WARN("SqlConnPool busy!");
        return nullptr;
    }
    sem_wait(&semId);    //信号量-1，若信号量的值为0,将被阻塞
    {
        std::lock_guard<std::mutex> locker(mtx);  //加锁  lock_guard出作用域直接解锁
        sql = connQue.front();
        connQue.pop();
    }
    return sql;
}

void WebServer::SqlConnPool::FreeConn(MYSQL* sql) {
    assert(sql);
    std::lock_guard<std::mutex> locker(mtx);
    connQue.push(sql);
    sem_post(&semId);    //信号量+1,信号量>0时，唤醒sem_wait;
}

void WebServer::SqlConnPool::ClosePool() {
    std::lock_guard<std::mutex> locker(mtx);
    while(!connQue.empty()) {
        auto item = connQue.front();
        connQue.pop();
        mysql_close(item);
    }
    mysql_library_end();        
}