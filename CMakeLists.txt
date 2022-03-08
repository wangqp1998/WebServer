cmake_minimum_required(VERSION 3.0)

project (WEBSERVER C CXX)


set(CMAKE_CXX_FLAGS "${CMKE_CXX_FLAGS} -g -Wall  -pthread ")




add_executable(WebServer main.cc
               src/timer/timer.cc 
               src/pool/sqlconnpool.cc
               src/Epoller/Epoller.cc 
               src/Http/HttpResponse.cc
               src/Http/HttpServer.cc
               src/Http/HttpRequest.cc
               src/Server/Server.cc 
               src/Socket/Socket.cc
               src/Buffer/Buffer.cc
               src/Log/Log.cc
)

link_directories(/usr/lib64/mysql/usr/lib/x86_64-linux-gnu)

target_link_libraries(WebServer libmysqlclient.so)


