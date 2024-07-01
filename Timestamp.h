#pragma once
#include <iostream>
#include <sys/timerfd.h>
#include <string>

class Timestamp{
private:
    time_t secsinceepoch_;//正数表示时间

public:
    Timestamp();
    Timestamp(int64_t secsinceepoch);

    static Timestamp now();//返回当前时间对象

    time_t toint()const;//返回整数表示时间
    std::string tostring()const;//返回字符串表示时间，yyyy-mm-dd hh(24):mi:ss 
};