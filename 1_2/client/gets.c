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

void getsFile(int peerfd)
{
    int clientfd = peerfd;

    //向服务端发送文件下载请求
    int ret = send(clientfd, (char*)&ret, sizeof(ret),0);

    //1.接收文件名的长度
    int len = 0;
    recv(clientfd, &len, sizeof(len), 0);
    printf("文件名长度:%d\n",len);
    
    //2.接收文件名
    char filename[100] = {0};
    recv(clientfd, filename, len, 0);
    filename[len] = '\0';
    printf("filename:%s\n",filename);

     // 检查文件是否存在
    if (access(filename, F_OK) != -1) {
        // 文件已存在，报错并返回
        fprintf(stderr, "文件 '%s' 已存在，无法覆盖。\n", filename);
        close(clientfd);
        return;
    }

    //3.接收文件的长度
    recv(clientfd, &len, sizeof(len), 0);
    printf("文件长度:%d\n", len);
    
    //4.接收文件
    //打印当前工作目录
    char cwd[128] = {0};
    getcwd(cwd, sizeof(cwd));
    printf("当前工作目录cwd: %s\n",cwd);


    int fd = open(filename, O_RDWR | O_CREAT , 0644); 
    printf("fd = %d\n",fd);
    if(fd == -1){    
        error(1, errno, "open %s", filename);
    }
    char buff[1000] = {0};
    off_t left = len;
    while(left > 0) {
        //可以确定接收len个字节的长度
        ret = recvn(clientfd, buff, len);//再接文件内容
        if(ret != 1000) {
            printf("ret: %d\n", ret);
        }
        //最后再写入本地
        write(fd, buff, ret);
        left-=ret;
    }
    close(fd);
    close(clientfd);
}
