#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <sys/stat.h>   //stat
#include <unordered_map>
#include <unistd.h>   //close
#include <fcntl.h>    //open
#include <sys/mman.h>


#include "../Log/Log.h"
#include "../Buffer/Buffer.h"


namespace WebServer
{
class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string& srcDir,std::string path,int code = -1,bool IsKeepAive= false);

    void MakeResponse(Buffer& buff);
    
    char* GetFile() {return mFile;}
    size_t GetFileSize() const {return mFileStat.st_size;}

    void UnmapFile();
    void ErrorContent(Buffer& buff, std::string message)  ;
private:
    void AddStateLine(Buffer &buff);
    void AddHeader(Buffer &buff);
    void AddContent(Buffer &buff);

     void ErrorHtml();

    std::string GetFileType();  /*获取文件类型*/
    
    int mycode;
    bool my_IsKeepAlive;

    std::string mypath;
    std::string mysrcDir;    

    char* mFile;
    struct stat mFileStat;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
};
}


#endif