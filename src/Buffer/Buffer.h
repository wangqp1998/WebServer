#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <string.h>
#include <string>
#include <sys/uio.h>   //iovec
#include <assert.h>


namespace WebServer
{

class Buffer
{
public:  
    static const size_t Size = 1024;
    Buffer(size_t initSize = Size):buffer(initSize),readerIndex(0),writerIndex(0)
    {
        assert(readableBytes() == 0);
        assert(writeableBytes() == initSize);
        assert(PrependableBytes() == 0);
    }   
    ~Buffer() {};

    ssize_t readFd(int fd,int* savedErrno);

    size_t readableBytes() const {return writerIndex - readerIndex;}
    const char* Peek() const {return begin()+readerIndex;}

    char* BeginWrite() {return begin()+writerIndex;}
    const char* BeginWriteConst() const{return begin()+writerIndex;}
    
    void Retrieve(size_t len) {assert(len <= readableBytes());readerIndex += len;}
    void RetrieveUntil(const char* end) {assert(Peek() <= end );Retrieve(end - Peek());}

    void RetrieveAll();     //清空
private:
   
    size_t writeableBytes() const {return buffer.size()-writerIndex;}
    size_t PrependableBytes() const {return readerIndex;}

   
    const char* begin() const{return &*buffer.begin();}
    char* begin() {return &*buffer.begin();}

    void Append(const char* str,size_t len);

    void EnsureWriteable(size_t len);

    void makeSpace(size_t len);
private:

    std::vector<char> buffer;
    size_t readerIndex;
    size_t writerIndex;    
};

}

#endif