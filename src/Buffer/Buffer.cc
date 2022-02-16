#include "Buffer.h"
#include <iostream>

ssize_t WebServer::Buffer::readFd(int fd,int* saveErrno)
{
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writeable = writeableBytes();
    vec[0].iov_base =begin()+writerIndex;
    vec[0].iov_len = writeable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    //如果buffer空间足够大,不使用extrabuf
    const int iovcnt = (writeable < sizeof extrabuf)?2:1;
    const ssize_t len = readv(fd,vec,iovcnt);
    if (len < 0)
        *saveErrno = errno;
    else if(static_cast<size_t>(len) <= writeable)
        writerIndex += len;
    else
    {
        writerIndex = buffer.size();
        Append(extrabuf,len - writeable);
    }
    //std::cout << buffer.data() << std::endl;
    return len;
}




void WebServer::Buffer::Append(const char* str,size_t len)
{
    EnsureWriteable(len);
    std::copy(str,str+len,BeginWrite());
    writerIndex += len;
}

void WebServer::Buffer::Append(const std::string &str)
{
    Append(str.data(),str.size());
}

void WebServer::Buffer::Append(const void* data, size_t len) {
    assert(data);
    Append(static_cast<const char*>(data), len);
}

void WebServer::Buffer::EnsureWriteable(size_t len)
{
    if(writeableBytes()<len)
        makeSpace(len);
    assert(writeableBytes()>=len);
}

void WebServer::Buffer::makeSpace(size_t len)
{
    if(writeableBytes()+PrependableBytes()<len)
    {
        buffer.resize(writerIndex+len);
    }
    else
    {
        size_t readable = readableBytes();
        std::copy(begin()+readerIndex,begin()+writerIndex,begin());
        readerIndex = 0;
        writerIndex = readerIndex + readable;
        assert(readable == readableBytes());
    }
}

void WebServer::Buffer::RetrieveAll()
{
    bzero(&buffer[0],buffer.size());
    readerIndex = 0;
    writerIndex = 0;
}