#include "thread_pool.h"

int sendn(int sockfd, const void* buff, int len) {
    // 确保将长度为len的数据全部发送完毕
    int left = len;
    const char* pbuf = (char*)buff;

    int ret = 0;
    while (left > 0) {
        ret = send(sockfd, pbuf, left, 0);
        ERROR_CHECK(ret, -1, "send");

        pbuf += ret;
        left -= ret;
    }

    return len - left;
}

int transferFile(task_t* task, long fileLength) {
    
    int peerfd = task->peerfd;
    char filename[128];
    strcpy(filename, task->data);

    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        error(1, 0, "Server: open file %s failed", filename);
    }

    train_t t;
    bzero(&t, sizeof(t));

    // 1. 先发送文件名
    t.len = strlen(filename);
    strcpy(t.buff, filename);
    send(peerfd, &t, 4 + t.len, 0);

    // 2. 发送要发送文件长度
    struct stat st;
    bzero(&st, sizeof(st));

    fstat(fd, &st);
    
    // 确定要发送文件的长度
    long sendLength = 0;
    sendLength = st.st_size - fileLength;
    send(peerfd, &sendLength, sizeof(sendLength), 0);

    // 3. 将文件的指移动到相应的位置
    int ret = lseek(fd, fileLength, SEEK_SET);
    if (ret == -1) {
        error(1, errno, "Server: lseek failed.");
    }
    
    // 4. 发送文件内容
    while (1) {
        bzero(&t, sizeof(t));
        ret = read(fd, t.buff, sizeof(t.buff));

        if (ret == 0) {
            break;
        }

        t.len = ret;
        ret = sendn(peerfd, &t, 4 + t.len);

        if (ret == -1) {
            break;
        }
    }
    
    close(fd);

    return 0;
}

void puts_breakpoint(task_t* task) {
    
    // 参数检验，确保task不为NULL
    assert(task);
    
    // 保存已发送的文件长度
    long fileLength = 0;

    // 保存客户端发送的文件名和peerfd
    char filename[128];
    strcpy(filename, task->data);
    int clientfd = task->peerfd;

    // 检查该文件是否存在
    if (access(filename, F_OK) == -1) {
        // 文件不存在
        const char* error = "File does not exist";
        send(clientfd, error, strlen(error), 0);
        return;
    }

    // 文件存在
    const char* ack = "File exists";
    send(clientfd, ack, sizeof(ack), 0);

    // 服务器端接受已发送文件长度
    int ret = recv(clientfd, &fileLength, sizeof(fileLength), 0);
    if (ret == -1) {
        error(1, 0, "Server: recv fileLength failed");
    }
    
    // 传输文件
    transferFile(task, fileLength);

    close(clientfd);

}
