#include "client.h"

int tcpConnect(const char *ip,unsigned short port){
    //创立TCP套接字
    int clientfd =socket(AF_INET,SOCK_STREAM,0);
    if(clientfd<0){
        perror("socket");
        return -1;
    }

    //填写服务器网络地址
    struct sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof(serveraddr));
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port=htons(port);
    serveraddr.sin_addr.s_addr=inet_addr(ip);

    //请求连接
    int ret=connect(clientfd,(const struct sockaddr*)&serveraddr,sizeof(serveraddr));
    if(ret<0)
    {
        perror("connect");
        close(clientfd);
        return -1;
    }
    return clientfd;
}
