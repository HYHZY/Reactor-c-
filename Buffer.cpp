#include "Buffer.h"


Buffer::Buffer(uint16_t seq):seq_(seq)
{

}
Buffer::~Buffer()
{

}
//添加数据
void Buffer::append(const char* data,size_t size)
{
    buf_.append(data,size);
}
void Buffer::appendwithseq(const char* data,size_t size)
{
    if(seq_==0)
    {
        buf_.append(data,size);
    }
    else if(seq_==1)
    {
        buf_.append((char*)&size,4);
        buf_.append(data,size);
    }
    else if(seq_==2)
    {

    }
}
//删除size字节数据
void Buffer::erase(size_t pos,size_t size)
{
    buf_.erase(pos,size);
}
//清空buf_
void Buffer::clear()
{
    buf_.clear();
}
//获取buf_大小
size_t Buffer::size()
{
    return buf_.size();
}
//获取data
const char* Buffer::data()
{
    return buf_.data();
}
//从buf_中拆分出一个报文，存放在ss中，如果没有报文，返回false
bool Buffer::pickmessage(std::string &ss)
{
    if(buf_.size()==0){
        return false;
    }
    if(seq_==0)
    {
        ss=buf_;
        return true;
    }
    else if(seq_==1)
    {
        int len;
        memcpy(&len,buf_.data(),4);
        if(buf_.size()<len+4) return false;
        ss=buf_.substr(4,len);
        buf_.erase(0,len+4);
    }
    else if(seq_==2)
    {

    }
    return true;
}

ssize_t Buffer::readfd(int fd,int* savedErrno)
{
    // char extrabuf[65536];
    // struct iovec vec[2];
    // const size_t writable=writableBytes();
    // ssize_t nread = read(fd, buf_.data(), BUFSIZ);
}