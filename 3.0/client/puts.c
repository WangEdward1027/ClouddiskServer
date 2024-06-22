/*王绪飞 2024.6.20*/
#include "client.h"

void putsCommand(int sockfd, train_t * pt)
{
    //上传文件，附加秒传功能:
    //先根据文件内容, 生成md5码, 客户端发送 用户名和md5码 给服务器
    //服务器根据用户名查 文件表:
    //若md5码值存在，则秒传成功
    //若用户名对应的md5值不存在,则开启普通传输
    
    char filename[20] = {0};
    removeTrailingSpace(pt->buff);
    strcpy(filename, pt->buff);
    //根据文件内容, 生成md5码, 客户端发送 md5码、用户名、文件名 给服务器
    char md5String[33];  // MD5 字符串长度为 32 个字符，再加上一个结束字符 \0
    generateMD5(filename, md5String);
    send(sockfd, md5String, strlen(md5String) + 1, 0);
    //还要传输文件名 --

    char response[256];
    receive_response(sockfd, response);
    if(strcmp(response, "instantTransfer") == 0){
        printf("文件秒传成功\n");
        return;
    }

    //服务器中没有该文件的md5码,开启普通的上传文件
    printf("开启普通上传文件\n");
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
