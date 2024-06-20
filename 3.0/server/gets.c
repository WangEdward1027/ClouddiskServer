#include "thread_pool.h"

void getsCommand(task_t* task) {
    
    // 服务端
    // 1. 查询文件是否存在，向客户端发送信息是否存在的响应
    // 2. 接收客户端发送的offset
    // 3. 向客户端发送文件
    
    char * filename = task->data;
    // 1. 查询文件是否存在
    int num = 1;
    int flag = 0; // 标志位，标记文件是否存在 0不存在，1存在
    FileEntry* file = selectFileEntryByFileNameAndOwnerId(filename, task->user->id, &num);
    if (file == NULL) {
        // 文件不存在，向客户端发送flag = 0
        flag = 0;
        sendn(task->peerfd, &flag, sizeof(flag));
        return;
    } else {
        // 文件存在，向客户端发送flag = 1
        flag = 1;
        sendn(task->peerfd, &flag, sizeof(flag));
    }

    // 2. 接收客户端发来的文件长度
    off_t offset;
    recvn(task->peerfd, &offset, sizeof(offset));

    // 3. 获取本地文件的长度
    int fd = open(filename, O_RDWR);
    struct stat st;
    bzero(&st, sizeof(st));
    fstat(fd, &st);
    
    int fileLength = st.st_size - offset;
    
    lseek(fd, offset, SEEK_SET);

    // 4.1 发送文件长度
    sendn(task->peerfd, &fileLength, sizeof(fileLength));

    // 4.2 发送文件内容
    // int ret = sendfile(task->peerfd, fd, &offset, fileLength);
    // printf("send %d bytes.\n", ret);
      
    train_t send_train;

    // 3. 向服务器发送文件
    while(1) {
        memset(&send_train, 0, sizeof(send_train));
        int ret = read(fd, send_train.buff, sizeof(send_train.buff));
        send_train.len = ret;
        sendn(task->peerfd, &send_train.len, sizeof(send_train.len));
        sendn(task->peerfd, send_train.buff, send_train.len);
        if(ret == 0){
            break;
        }
    }
}   
