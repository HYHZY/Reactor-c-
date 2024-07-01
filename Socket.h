#pragma once

#include <sys/socket.h>
#include <sys/types.h>          
#include <sys/fcntl.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>      // TCP_NODELAY需要包含这个头文件。
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "InetAddress.h"
using SA = struct sockaddr;
int createnonblocking();
class Socket{
private:
    const int fd_;
    std::string ip_;
    uint16_t port_;
public:
    Socket(int fd);
    ~Socket();

    int fd()const;
    std::string ip()const;
    uint16_t port()const;
    void setipport(const std::string &ip,uint16_t port);

    void setreuseaddr(bool on);
    void setreuseport(bool on);
    void settcpnodelay(bool on);
    void setkeepalive(bool on);
    void Bind(const InetAddress& servaddr);
    void Listen(int n=128);
    int accept(InetAddress& clientaddr);

};