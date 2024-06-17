#include "client.h"

//客服端接收服务器发送的内容

//接收确定个字节的数据
int recvn(int sockfd, void * buff, int len)
{
    int left = len;
    char * pbuf = (char*)buff;
    int ret = 0;
    while(left > 0) {
        ret = recv(sockfd, pbuf, left, 0);
        if(ret < 0) {
            perror("recv");
            break;
        } else if (ret == 0) {
            break;
        }
        pbuf += ret;
        left -= ret;
    }
    return len - left;
}

void getsCommand(task_t* task)
{
    int clientfd = task->peerfd;
    int ret = 1;

    //向客户端发送文件可接收请求
    ret = send(clientfd, (char*)&ret, sizeof(ret),0);

    //先接收文件名
    char filename[100] = {0};
    int len = 0;
    ret = recvn(clientfd, (char*)&len, sizeof(len));//先接文件名长度
    printf("ret: %d, filename's len:%d\n", ret, len);
    ret = recvn(clientfd, filename, len);//再接文件名内容
    printf("ret: %d, recv msg:%s\n", ret, filename);

    int wfd = open(filename, O_CREAT | O_RDWR, 0644);

    //再获取的是文件内容的长度
    off_t length = 0;
    recvn(clientfd, (char*)&length, sizeof(length));
    printf("file length: %ld\n", length);

    //最后接收文件内容
    char buff[1000] = {0};
    while(1) {
        ret = recvn(clientfd, (char*)&len, sizeof(len));//先接长度
        if(ret == 0) {
            break;
        }
        //可以确定接收len个字节的长度
        ret = recvn(clientfd, buff, len);//再接文件内容
        if(ret != 1000) {
            printf("ret: %d\n", ret);
        }
        //最后再写入本地
        write(wfd, buff, ret);
    }
    close(wfd);
    close(clientfd);
}
