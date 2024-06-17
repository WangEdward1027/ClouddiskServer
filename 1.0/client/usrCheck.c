#include "client.h"

//客户端发送信息
int sendMessage(int sockfd, char* buffer, CmdType cmdType){

    int ret;
    int len = strlen(buffer);
    //1.1先发消息长度
    ret = sendn(sockfd, (char*)&len, sizeof(len));
    ERROR_CHECK(ret, -1, "sendMessage len");
    //1.2再发消息类型
    ret = sendn(sockfd, &cmdType, sizeof(CmdType));
    ERROR_CHECK(ret, -1, "sendMessage CmdType");
    //1.3最后发消息内容
    ret = sendn(sockfd, buffer, len);
    ERROR_CHECK(ret , -1, "sendMessage data");

    return 0;
}

//客户端接收信息
void recvMessage(int sockfd, char* buffer){
    //1.1 获取消息长度
    int length = -1;
    int ret = recvn(sockfd, &length, sizeof(length));
    printf("recv length: %d\n", length);

    //1.2 获取消息类型
    CmdType cmdType;
    ret = recvn(sockfd, &cmdType, sizeof(cmdType));
    printf("recv cmd type: %d\n", cmdType);
    //1.3 获取消息内容
    ret = recvn(sockfd, buffer, length);
}

//用户校验
int usrCheck(int sockfd)
{
    //客户端输入用户名发送给服务器
    char usrname[128];
    printf("请输入登录用户名：");
    scanf("%s",usrname);
    sendMessage(sockfd, usrname, CMD_TYPE_USRNAME);
    
    //接收服务器端返回的盐值
    char setting[128] = {0};
    recvMessage(sockfd, setting);
    
    //客户输入密码
    char password[128] = {0};
    printf("\n请输入密码：");
    scanf("%s",password);
    
    //将密码与盐值一起加密发送给服务器
    char* encoded = crypt(password, setting);
    sendMessage(sockfd, encoded, CMD_TYPE_USRENCODE);
    
    //接收服务器的校验结果
    int result;
    recvMessage(sockfd, (char*)&result);

    return result;
}

