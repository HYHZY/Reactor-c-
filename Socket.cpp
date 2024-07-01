#include "Socket.h"

int createnonblocking(){
    // 创建服务端用于监听的listenfd。
    int listenfd = socket(AF_INET,SOCK_STREAM|O_NONBLOCK,IPPROTO_TCP);
    if (listenfd < 0)
    {
        // perror("socket() failed"); 
        printf("%s:%s:%d listen socket create error:%d\n",__FILE__,__FUNCTION__,__LINE__,errno);
        exit(-1);
    }
    return listenfd;
}
Socket::Socket(int fd):fd_(fd){
    
}
Socket::~Socket(){
    ::close(fd_);
}
int Socket::fd()const{
    return fd_;
}
std::string Socket::ip()const
{
    return ip_;
}
uint16_t Socket::port()const
{
    return port_;
}
void Socket::setreuseaddr(bool on){
    int opt = on?1:0; 
    setsockopt(fd_,SOL_SOCKET,SO_REUSEADDR,&opt,static_cast<socklen_t>(sizeof opt));    // 必须的。
}
void Socket::setreuseport(bool on){
    int opt = on?1:0; 
    setsockopt(fd_,SOL_SOCKET,TCP_NODELAY   ,&opt,static_cast<socklen_t>(sizeof opt));    // 必须的。
}
void Socket::settcpnodelay(bool on){
    int opt = on?1:0; 
    setsockopt(fd_,SOL_SOCKET,SO_REUSEPORT ,&opt,static_cast<socklen_t>(sizeof opt));    // 有用，但是，在Reactor中意义不大。
}
void Socket::setkeepalive(bool on){
    int opt = on?1:0; 
    setsockopt(fd_,SOL_SOCKET,SO_KEEPALIVE   ,&opt,static_cast<socklen_t>(sizeof opt));    // 可能有用，但是，建议自己做心跳。
}
void Socket::Bind(const InetAddress& servaddr){
    if(::bind(fd_,servaddr.addr(),sizeof(SA))!=0){
        perror("bind() failed"); close(fd_); exit(-1);
    }
    ip_=servaddr.ip();
    port_=servaddr.port();
}

void Socket::Listen(int n){
    if (listen(fd_,128) != 0 )        // 在高并发的网络服务器中，第二个参数要大一些。
    {
        perror("listen() failed"); close(fd_); exit(-1);
    }
}
int Socket::accept(InetAddress& clientaddr){
    struct sockaddr_in peeraddr;
    socklen_t len = sizeof(peeraddr);
    int clientfd = accept4(fd_,(SA*)&peeraddr,&len,O_NONBLOCK);
    clientaddr.setaddr(peeraddr);
    
    return clientfd;
}

void Socket::setipport(const std::string &ip,uint16_t port)   
{
    ip_=ip;
    port_=port;
}


