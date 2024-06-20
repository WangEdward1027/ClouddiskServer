//王绪飞:客服端接收服务器发送的内容

#include "client.h"

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
    int len = 0;    //len是文件名的长度
    recv(clientfd, &len, sizeof(len), 0);
    printf("文件名长度:%d\n",len);
    
    //2.接收文件名
    char filename[20] = {0};
    recv(clientfd, filename, len, 0);
    filename[len] = '\0';
    printf("filename:%s\n",filename);

    //3.接收文件的长度
    recv(clientfd, &len, sizeof(len), 0);
    printf("文件长度:%d\n", len);
    
    //4.接收文件
        //打印当前工作目录
        char cwd[128] = {0};
        getcwd(cwd, sizeof(cwd));
        printf("当前工作目录cwd: %s\n",cwd);
    int filefd = open(filename, O_RDWR | O_CREAT |O_TRUNC , 0644); 
    if(filefd == -1)    error(1, errno, "open %s", filename);

    char buff[1000] = {0};
    recv(peerfd, buff, sizeof(buff), 0);
    write(filefd, buff, len);

    /* off_t left = len; */
    /* while(left > 0) { */
    /*     //可以确定接收len个字节的长度 */
    /*     ret = recvn(clientfd, buff, len);//再接文件内容 */
    /*     if(ret != 1000) { */
    /*         printf("ret: %d\n", ret); */
    /*     } */
    /*     //最后再写入本地 */
    /*     write(fd, buff, ret); */
    /*     left -= ret; */
    /* } */

    close(filefd);
    close(clientfd);
}
