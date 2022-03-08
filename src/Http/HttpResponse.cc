#include "HttpResponse.h"

const std::unordered_map<std::string, std::string> WebServer::HttpResponse::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
    //application/yuml   浏览器会下载文件
};

const std::unordered_map<int, std::string> WebServer::HttpResponse::CODE_STATUS = {
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

const std::unordered_map<int, std::string> WebServer::HttpResponse::CODE_PATH = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};



WebServer::HttpResponse::HttpResponse()
    :mycode(-1),mypath(""),my_IsKeepAlive(false),mysrcDir(""),mFile(nullptr),mFileStat{0}
{

}

WebServer::HttpResponse::~HttpResponse()
{
    UnmapFile();
}

void WebServer::HttpResponse::MakeResponse(Buffer& buff)
{
    if(stat((mysrcDir + mypath).data(), &mFileStat) < 0 || S_ISDIR(mFileStat.st_mode)) {
        mycode = 404;
    }
    else if(!(mFileStat.st_mode & S_IROTH)) {
        mycode = 403;
    }
    else if(mycode == -1) { 
        mycode = 200; 
    }
    ErrorHtml();
    AddStateLine(buff);
    AddHeader(buff);
    AddContent(buff);
} 

void WebServer::HttpResponse::ErrorHtml() {
    if(CODE_PATH.count(mycode) == 1) {
        mypath = CODE_PATH.find(mycode)->second;
        stat((mysrcDir + mypath).data(), &mFileStat);
    }
}

void WebServer::HttpResponse::Init(const std::string& srcDir_,std::string path,int code ,bool IsKeepAive)
{
    mycode=code;
    my_IsKeepAlive=IsKeepAive;
    mypath=path;
    mysrcDir=srcDir_;
    mFile = nullptr;
    mFileStat = {0};
    LOG_DEBUG("IsKeepAlive:%d",my_IsKeepAlive);
}

void WebServer::HttpResponse::AddStateLine(Buffer &buff)
{
    std::string status;
    if(CODE_STATUS.count(mycode) == 1)
    {   
        status = CODE_STATUS.find(mycode) -> second;
    }
    else 
    {
        mycode = 400;
        status = CODE_STATUS.find(400) -> second;
    }
    buff.Append("HTTP/1.1 " + std::to_string(mycode) + " " + status + "\r\n");
}
void WebServer::HttpResponse::AddHeader(Buffer &buff)
{
    buff.Append("Connection: ");
    if(my_IsKeepAlive)
    {
        buff.Append("keep-alive\r\n");
        buff.Append("keep-alive: max=6, timeout=120\r\n");
    }
    else    
        buff.Append("close\r\n");
    buff.Append("Content-type: " + GetFileType() + "\r\n");
}
void WebServer::HttpResponse::AddContent(Buffer &buff)
{
    int srcFd = open((mysrcDir + mypath).data(),O_RDONLY);
    if(srcFd < 0)
    {
        //报错
        return;
    }
    mFileStat.st_size = lseek(srcFd,0,SEEK_END);
    mFile = (char*)mmap(0, mFileStat.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);

    close(srcFd);
    buff.Append("Content-length: " + std::to_string(mFileStat.st_size) + "\r\n\r\n");
}

std::string WebServer::HttpResponse::GetFileType()
{
    std::string::size_type idx = mypath.find_last_of('.');//找最后一个点
    if(idx == std::string::npos)
    {
        return "text/plain";
    }
    std::string suffix = mypath.substr(idx);
    if(SUFFIX_TYPE.count(suffix) == 1)
    {
        return SUFFIX_TYPE.find(suffix) -> second;
    }
    return "text/plain";
}

void WebServer::HttpResponse::UnmapFile()
{
    if(mFile)
    {
        munmap(mFile,mFileStat.st_size);
        mFile = nullptr;
    }
}

void WebServer::HttpResponse::ErrorContent(Buffer& buff, std::string message) 
{
    std::string body;
    std::string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if(CODE_STATUS.count(mycode) == 1) {
        status = CODE_STATUS.find(mycode)->second;
    } else {
        status = "Bad Request";
    }
    body += std::to_string(mycode) + " : " + status  + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buff.Append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}