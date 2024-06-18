#include "client.h"

int sendn(int sockfd, const void* buff, int len) {
    
     int left = len;
     const char* pbuf = (char*)buff;
     int ret = 0;
     while (left > 0) {
        ret = send(sockfd, pbuf, left, 0);
        ERROR_CHECK(ret, -1, "send");

        pbuf += ret;
        left -= ret;
    }
    return 0;
}

//将文件上传到服务器
void putsFile(int sockfd, train_t * pt)
{
    char filename[20] = {0};
    strcpy(filename, pt->buff);
    printf("filename:%s\n",filename);

    //打开文件
    int fd = open(filename, O_RDWR);
    if(fd < 0) {
        perror("open"); return;
    }
    //获取文件大小
    struct stat st;
    memset(&st, 0, sizeof(st));
    fstat(fd, &st);
    printf("file length: %ld\n", st.st_size);
    //发送文件大小
    sendn(sockfd, &st.st_size, sizeof(st.st_size));
    off_t cur = 0;
    char buff[1000] = {0};
    int ret = 0;
    //发送内容
    while(cur < st.st_size) {
        memset(buff, 0, sizeof(buff));
        ret = read(fd, buff, sizeof(buff));
        if(ret == 0) {
            break;
        }
        ret = sendn(sockfd, buff, ret);
        cur +=  ret;
    }
    //发送完成
    printf("file send over.\n");
    close(fd);
}

