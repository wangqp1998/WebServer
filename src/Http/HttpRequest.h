#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "../Buffer/Buffer.h"
#include <string>
#include <regex>
#include <algorithm>
#include <unordered_map>

namespace WebServer
{
class HttpRequest
{
public:
    enum PARSE_STATE{
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };
    enum HTTP_CODE{
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION, 
    };

    HttpRequest() {};
    ~HttpRequest() {};

    void init();

    bool pares(Buffer& buff);
    
    bool IsKeepAlive() const;

    std::string Getpath() {return path;}

private:
    bool ParseRequestLine(const std::string& line);
    void ParseHeader(const std::string& line);
    void ParseBody(const std::string& line);

    void ParsePath();
    PARSE_STATE state;

    std::string method,path,version,body;
    std::unordered_map<std::string,std::string> headers;
};
}

#endif