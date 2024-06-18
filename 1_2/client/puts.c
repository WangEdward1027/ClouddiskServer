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
void putsFile(int sockfd, char* filename)
{
    //参数校验
    /* assert(task); */

    //读取本地文件
    int fd = open(filename, O_RDWR);

    train_t t;
    memset(&t, 0, sizeof(t));

    //先发送文件名
    t.len = strlen(filename);
    strcpy(t.buff, filename);
    send(sockfd, &t, 4 + t.len, 0);

    //其次发送文件长度
    struct stat st;
    memset(&st, 0, sizeof(st));
    fstat(fd, &st);
    printf("filelength: %ld\n", st.st_size);//off_t

    send(sockfd, &st.st_size, sizeof(st.st_size), 0);

    //最后发送文件内容
    while(1) {
        memset(&t, 0, sizeof(t));
        int ret = read(fd, t.buff, sizeof(t.buff));
        if(ret != 1000) {
            printf("read ret: %d\n", ret);
        }
        if(ret == 0) {
            //文件已经读取完毕
            break;
        }
        t.len = ret;
        ret = sendn(sockfd, &t, 4 + t.len);
        if(ret == -1) {
            break;
        }
        if(ret != 1004) {
            printf("send ret: %d\n", ret);
        }
    }
    close(fd);//关闭文件
}

