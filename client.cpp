// 网络通讯的客户端程序。
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#include "comFun.h"
using namespace std;
using SA = struct sockaddr;

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in servaddr;
    char buf[BUFSIZE];
 
    if ((sockfd=socket(AF_INET,SOCK_STREAM,0))<0) { printf("socket() failed.\n"); return -1; }
    
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(PORT);
    servaddr.sin_addr.s_addr=inet_addr(IP);

    if (connect(sockfd, (SA*)&servaddr,sizeof(servaddr)) != 0)
    {
        printf("connect(%s:%s) failed.\n",argv[1],argv[2]); close(sockfd);  return -1;
    }

    printf("connect ok.\n");
    // printf("开始时间：%d",time(0));
    for (int i=0;i<10;i++)
    {
        // 从命令行输入内容。
        memset(buf,0,sizeof(buf));
        sprintf(buf,"this is %d ironman.",i); 
        char tmpbuf[BUFSIZE];
        memset(tmpbuf,0,sizeof(tmpbuf));
        int len=strlen(buf);
        memcpy(tmpbuf,&len,4);//拼接头部长度
        memcpy(tmpbuf+4,buf,len);//拼接报文
        send(sockfd,tmpbuf,len+4,0);
    }
    for(int i=0;i<10;++i){
        int len;
        recv(sockfd,&len,4,0);            // 先读取4字节的报文头部。

        memset(buf,0,sizeof(buf));
        recv(sockfd,buf,len,0);           // 读取报文内容。

        printf("recv:%s\n",buf);
    }
    // printf("结束时间：%d",time(0));
    return 0;
} 