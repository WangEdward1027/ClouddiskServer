#include "client.h"

void getsCommand(int sockfd, train_t * pt) {

    // 客户端
    // 1. 接受客户端发送的内容，查看是否存在
    // 2. 文件存在，查询本地是否存在文件，发送offset
    // 3. 接收客户端发送的内容
    removeTrailingSpace(pt->buff);

    char* filename = pt->buff;  
    int fd; // 文件描述符
    // 1. 接受服务器发送的文件是否存在的标记
    int flag;
    recvn(sockfd, &flag, sizeof(flag));

    // 1.1 文件不存在
    if (flag == 0) {
        printf("目标文件不存在!\n");
        return;
    } 
    
    // 服务器端文件存在
    // 2. 查看本地文件是否存在文件
    int offset;
    if (access(filename, F_OK) != 0) {
        // 本地文件不存在
        offset = 0;
        fd = open(filename, O_RDWR | O_CREAT, 0664);
    } else {
        // 本地文件存在，读取文件大小
        struct stat st;
        bzero(&st, sizeof(st));
        fd = open(filename, O_RDWR | O_APPEND);
        fstat(fd, &st);
        offset = st.st_size;
    }
    // 2.1 向服务器发送文件的大小
    sendn(sockfd, &offset, sizeof(offset));
    
    // 3. 接受客户端发送的文件长度
    int fileLength;
    recvn(sockfd, &fileLength, sizeof(fileLength));

    // 3.1 进度条
    // off_t splice = fileLength / 100;
    // off_t currSize = 0;
    // off_t lastSize = 0;
    
    // 3.2 接收文件内容
    char buff[1000] = {0};
    while (1) {
        int len;
        int ret = recvn(sockfd, &len, sizeof(len));
        if(len == 0) {
            break;
        }
        memset(buff, 0, sizeof(buff));
        ret = recvn(sockfd, buff, len);
        if(ret != sizeof(buff)) {
            printf("ret: %d\n", ret);
        }
        // printf("接收到的文件内容是：%s", buff);
        write(sockfd, buff, len);
    }
}
