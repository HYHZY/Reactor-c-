#pragma once
#include <string>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/uio.h>

class Buffer{
private:
    std::string buf_;//存放数据
    uint16_t seq_;//分隔符：1：头部加4B长度字段，0：无分隔符（视频会议），2：“\r\n\r\n”（http）
public:
    Buffer(uint16_t seq=1);
    ~Buffer();

    void append(const char* data,size_t size);//添加数据
    void clear();//清空buf_
    void erase(size_t pos,size_t size);//删除从pos开始的size字节数据
    size_t size();//获取buf_大小
    const char* data();//获取data
    void appendwithseq(const char* data,size_t size);//添加头部长度字段
    bool pickmessage(std::string &ss);//从buf_中拆分出一个报文，存放在ss中，如果没有报文，返回false

    ssize_t readfd(int fd,int* savedErrno);//直接将fd数据读到buf_中
};