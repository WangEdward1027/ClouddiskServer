#include "client.h"

void getsCommand(int sockfd, train_t * pt) {

    // 客户端
    // 1. 接受客户端发送的内容，查看是否存在
    // 2. 文件存在，查询本地是否存在文件，发送offset
    // 3. 接收客户端发送的内容
    removeTrailingSpace(pt->buff);

    char* filename = pt->buff;  
    
    // 1. 接受服务器发送的文件是否存在的标记
    int flag;
    recvn(sockfd, &flag, sizeof(flag));
    // printf("客户端收到的标志位是:%d", flag);

    // 1.1 flag = 0 文件不存在
    if (flag == 0) {
        printf("目标文件不存在!\n");
        return;
    } 
    
    // flag = 1 服务器端文件存在
    
    // 2. 接受客户端发送的文件长度
    int fileLength;
    recvn(sockfd, &fileLength, sizeof(fileLength));

    // 3.1 进度条
    // off_t splice = fileLength / 100;
    // off_t currSize = 0;
    // off_t lastSize = 0;
    
    // 2.1 打开文件
    int fd = open(filename, O_RDWR | O_TRUNC | O_CREAT, 0664);

    // 3 接收文件内容
    char buff[1000] = {0};
    int ret = 0;
    int left = fileLength;
    while (left > 0) {
        if (left < 1000) {
            ret = recvn(sockfd, buff, left);
        } else {
            ret = recvn(sockfd, buff, sizeof(buff));
        }
        if (ret < 0) {
            break;
        }
        ret = write(fd, buff, ret);
        left -= ret;
    }
    close(fd);

    printf("接收完毕！\n");
}
