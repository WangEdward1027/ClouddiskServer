#include "client.h"

int main()
{
    //创建客户端套接字
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(clientfd, -1, "socket");

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));//初始化
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(8080);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = connect(clientfd, 
                      (const struct sockaddr *)&serveraddr, 
                      sizeof(serveraddr));
    ERROR_CHECK(ret, -1, "connect");

    //用户身份校验
    //输入用户名发送给服务端
    char usrname[128];
    
    //获取setting盐值
    
    //输入密码和盐值一起加密发送给服务端
    
    //服务端返回校验结果
    
    //用户输入命令分词执行



    return 0;
}

