#include "HttpRequest.h"

void WebServer::HttpRequest::init()
{
    method=path=version=body="";
    state = REQUEST_LINE;
    headers.clear();
    //post.clear();
}



bool WebServer::HttpRequest::IsKeepAlive() const
{
    if(headers.count("Connetion") == 1)
    {
        return headers.find("Connection")->second == "keep-alive" && version == "1.1";
    }
    return false;
}

bool WebServer::HttpRequest::pares(Buffer& buff)
{
    const char CRLF[]="\r\n";
    if(buff.readableBytes() <= 0)
        return false;
    while (buff.readableBytes() && state != FINISH)  //能读并且没读完
    {
      const char* lineEnd = std::search(buff.Peek(), buff.BeginWriteConst(), CRLF, CRLF + 2);
      std::string line(buff.Peek(),lineEnd);
      switch (state)
      {
        case REQUEST_LINE:
            if(!ParseRequestLine(line))
                return false;
            ParsePath();
            break;
        case HEADERS:
            ParseHeader(line);
            if(buff.readableBytes()<=2)
            {
                state=FINISH;
            }
            break;
        case BODY:
            ParseBody(line);
            break;
        default:
            break;
      }
        if(lineEnd == buff.BeginWrite()) { break; }
        buff.RetrieveUntil(lineEnd + 2);
    }
   return true; 
}

bool WebServer::HttpRequest::ParseRequestLine(const std::string& line)
{
    //GET / HTTP/1.1
    std::regex patten("([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch subMatch;
    if(std::regex_match(line,subMatch,patten))
    {
        method = subMatch[1];
        path = subMatch[2];
        version = subMatch[3];
        state = HEADERS;
        return true;
    }
    return false;
}
void WebServer::HttpRequest::ParsePath()
{
    if(path == "/")
    {
        path = "/index.html"; 
    }
    else
    {
        path = "/index.html";   /*测试，需修改*/ 
    }
}
void WebServer::HttpRequest::ParseHeader(const std::string& line)
{
     //Host: localhost:9600
    std::regex patten("([^ ]*): ?(.*)$");
    std::smatch subMatch;
    if(std::regex_match(line,subMatch,patten))
    {
       headers[subMatch[1]]=subMatch[2];
    }
    else
    {
        state = BODY;
    }
}
void WebServer::HttpRequest::ParseBody(const std::string& line)
{
    body = line;
    //ParsePost_();
    state = FINISH;
}