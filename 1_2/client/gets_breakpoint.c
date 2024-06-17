#include "client.h"

int recvn(int sockfd, void* buff, int len) {
    int left = len;
    char* pbuf = (char*)buff;
    int ret = 0;
    
    while (left > 0) {
        ret = recv(sockfd, pbuf, left, 0);
        if (ret < 0) {
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

int recvFile(int peerfd) {

    int clientfd = peerfd;
    char filename[128];
    bzero(&filename, sizeof(filename));
    
    int len = 0;
    int ret = 0;

    // 1. 先接收文件名
    ret = recv(clientfd, &len, sizeof(len), 0); // length
    ret = recv(clientfd, &filename, len, 0); // content
    
    // 2. 打开文件
    int wfd = open(filename, O_CREAT | O_APPEND, 0664);
    if (wfd == -1) {
        error(1, errno, "Client: open file failed");
    }

    // 3. 接收，发送文件的长度
    off_t length = 0;
    recv(clientfd, &length, sizeof(length), 0);

    // 4. 接受文件内容
    char buff[1000] = {0};
    
    while (1) {
        // recv file segment length 
        ret = recvn(clientfd, (char*)&len, sizeof(len));
        if (ret == 0) {
            break;
        }
        // recv file segment content
        ret = recvn(clientfd, buff, len);
        
        write(wfd, buff, ret);
    }

    close(wfd);
    return 0;
}

void get_breakpoint(int peerfd, char* filename) {

    char response[128];
    bzero(&response, sizeof(response));
    long fileLength = 0;
        
    // 接受服务器端的对文件是否存在的返回
    int ret =recv(peerfd, &response, sizeof(response), 0);
    if (ret == -1) {
        error(1, errno, "Client: recv response failed");
    }
    
    if (strcmp(response, "File exists") == 0) {
        // 如果文件存在
        // 获取已接收文件长度
        struct stat st;
        bzero(&st, sizeof(st));
        
        int fd = open(filename, O_RDWR);
        fstat(fd, &st);
        
        fileLength = st.st_size;
    } else {
        // 之前未接受过文件，已接收文件长度设为0
        fileLength = 0;
    }

    // 发送已接收文件长度
    ret = send(peerfd, &fileLength, sizeof(fileLength), 0);
    if (ret == -1) {
        error(1, errno, "Client: send fileLength failed!");
    }
    
    // 接受文件
    recvFile(peerfd);

    close(peerfd);

    
}
