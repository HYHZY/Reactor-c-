#include "Timestamp.h"

Timestamp::Timestamp(){
    secsinceepoch_=time(0);
}
Timestamp::Timestamp(int64_t secsinceepoch)
:secsinceepoch_(secsinceepoch)
{

}
//返回当前时间对象
Timestamp Timestamp::now()
{
    return Timestamp();
}
//返回整数表示时间
time_t Timestamp::toint()const
{
    return secsinceepoch_;
}
//返回字符串表示时间，yyyy-mm-dd hh(24):mi:ss 
std::string Timestamp::tostring() const
{
    char buf[128]={0};
    tm *tm_time=localtime(&secsinceepoch_);
    snprintf(buf,128,"%4d-%02d-%02d %02d:%02d:%02d",
            tm_time->tm_year+1900,
            tm_time->tm_mon+1,
            tm_time->tm_mday,
            tm_time->tm_hour,
            tm_time->tm_min,
            tm_time->tm_sec
    );
    return buf;
}