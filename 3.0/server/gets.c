#include "thread_pool.h"

void getsCommand(task_t* task) {
    
    // 服务端
    // 1. 查询文件是否存在，向客户端发送信息是否存在的响应
    // 2. 接收客户端发送的offset
    // 3. 向客户端发送文件
    removeTrailingSpace(task->data);
    
    char * filename = task->data;
    
    // 1. 查询文件是否存在
    
    // 1.1 根据用户名找到用户对应的user表的信息
    char * username = task->user->userName;
    User * user = selectUserByUserName(username);
    // char * pwd = user->pwd;

    // 1.2 根据用户id和文件名查找文件是否存在
    int num = 1;
    int flag = 0; // 标志位，标记文件是否存在 0不存在，1存在
    FileEntry* file = selectFileEntryByFileNameAndOwnerId(filename, user->id, &num);
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

    // 1.3 因为使用了文件秒传，使用MD5标记文件的信息，
    // 需要根据虚拟文件表MD5的信息查询真正的文件名。
    FileInfo * real_file = selectFileInfo(file->md5, 32);
    filename = real_file->fileName;

    // 2. 获取文件长度
    int fd = open(filename, O_RDWR);
    struct stat st;
    bzero(&st, sizeof(st));
    fstat(fd, &st);
    
    int fileLength = st.st_size;
    
    // 2.1 发送文件长度
    sendn(task->peerfd, &fileLength, sizeof(fileLength));

    
    // 3. 向服务器发送文件内容
    char buff[1000];
    int curr = 0;
    int ret = 0;
    while(curr < fileLength) {
        bzero(buff, sizeof(buff));
        ret = read(fd, buff, sizeof(buff));
        if (ret == 0) {
            break;
        }
        ret = sendn(task->peerfd, buff, ret);
        curr += ret;
    }
    close(fd);
    printf("发送完毕!\n");
    return;
}   
