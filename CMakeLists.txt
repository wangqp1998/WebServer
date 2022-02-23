cmake_minimum_required(VERSION 3.0)

project (WEBSERVER C CXX)

set(CMAKE_CXX_FLAGS "${CMKE_CXX_FLAGS} -g  -Wall -pthread")

#include_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(WebServer main.cc 
               src/Epoller/Epoller.cc 
               src/Http/HttpResponse.cc
               src/Http/HttpServer.cc
               src/Http/HttpRequest.cc
               src/Server/Server.cc 
               src/Socket/Socket.cc
               src/Buffer/Buffer.cc
               src/Log/Log.cc
                                    )

