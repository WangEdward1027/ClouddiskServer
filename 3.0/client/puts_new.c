#include "client.h"

// 客户端
void putsCommand_new(int sockfd, train_t * pt) {
    
    char response[64] = {0};
    // 1. 客户端先发送md5的值给服务端
    // 1.1 获取要上传文件的文件名
    char filename[20] = {0};
    removeTrailingSpace(pt->buff);
    strcpy(filename, pt->buff);

    // 1.2 根据文件内容，生成md5值
    char md5String[33];
    generateMD5(filename, md5String);

    
    // 1.3 发送给服务端md5值
    sendn(sockfd, md5String, sizeof(md5String));
    printf("md5码值为：%s\n",md5String);
    
    // 2. 接受客户端返回的结果
    int ret = -1;
    recvn(sockfd, &ret, sizeof(int));
    printf("客户端收到的结果是%d\n", ret);

    // 2.1 文件存在ret = 1，实现文件秒传;
    if (ret == 1) {
        // 接受客户端发送的响应信息
        int reslength;
        recvn(sockfd, &reslength, sizeof(int));
        printf("%d\n", reslength);
        recvn(sockfd, response, reslength);
        printf("%s\n", response);
        return;
    }
    // 3. 文件不存在ret = 0,向客户端发送文件名，文件长度，文件内容
    else if (ret == 0) {
        // 3.1 向服务器发送文件长度
        // 3.1.1 获取文件大小
        int fd = open(filename, O_RDWR);
        struct stat st;
        memset(&st, 0, sizeof(st));
        fstat(fd, &st);
        long int filelength = st.st_size;
        printf("file length: %ld\n", filelength);
        // 3.1.2 发送文件大小
        sendn(sockfd, &filelength, sizeof(filelength));
        
        // 3.2 发送文件内容
        char buff[1000];
        off_t curr = 0;
        int ret = 0;
        while (curr < filelength) {
            bzero(buff, sizeof(buff));
            ret = read(fd, buff, sizeof(buff));
            if (ret == 0) {
                break;
            }
            ret = sendn(sockfd, buff, ret);
            curr += ret;
        }
        close(fd);
        
        // 3.3 接受服务器返回的信息
        recvn(sockfd, response, sizeof(response));
        printf("%s\n", response);
        return;

    }

}
