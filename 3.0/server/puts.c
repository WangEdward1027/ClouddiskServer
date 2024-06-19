#include "thread_pool.h"

void putsCommand(task_t * task) {
    printf("execute puts command.\n");
    char filename[20] = {0};
    strcpy(filename, task->data);
    printf("filname: %s\n", filename);
    off_t len = 0;
    int ret = recvn(task->peerfd, &len, sizeof(len));
    printf("filelen: %ld.\n", len);

    //打开文件
    int fd = open(filename, O_CREAT|O_RDWR, 0644);
    if(fd < 0) {
        perror("open"); return;
    }
    //接收并写入文件
    char buff[1000] = {0};
    off_t left = len;
    while(left > 0) {
        if(left < 1000) {
            ret = recvn(task->peerfd, buff, left);
        } else {
            ret = recvn(task->peerfd, buff, sizeof(buff));
        }
        if(ret < 0) {
            break;
        }
        ret = write(fd, buff, ret);
        left -= ret;
    }
    close(fd);
    //上传任务执行完毕之后，再加回来
    addEpollReadfd(task->epfd, task->peerfd);
}
