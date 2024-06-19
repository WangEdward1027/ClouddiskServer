#include "thread_pool.h"

void putsCommand(task_t* task)
{
    send(task->peerfd,"do",2,0);
    printf("execute puts command.\n");
    char filename[20] = {0};
    strcpy(filename, task->data);
    printf("filname: %s\n", filename);
    off_t len = 0;
    int ret = recvn(task->peerfd, &len, sizeof(len));
    printf("filelen: %ld.\n", len);
    
    //打开文件
    int fd = open(filename, O_RDWR|O_CREAT, 0644);
    printf("%d",fd);
    if(fd < 0) {
        const char *msg="目录不存在";
        sendn(task->peerfd,msg,strlen(msg));
        perror("open"); 
        return;
    }
    //接收并写入文件
    char buff[1000] = {0};
    off_t left = len;
    printf("left:%ld\n",left);
    while(left > 0) {
        if(left < 1000) {
            
            ret = recvn(task->peerfd, buff, left);
            printf("发送1000字节\n");
        } else {

            ret = recvn(task->peerfd, buff, sizeof(buff));
            printf("少于一千字节\n");    
        }
        if(ret < 0) {
            break;
        }
        ret = write(fd, buff, ret);
        printf("ret:%d\n",ret);

        left -= ret;
    }
    const char *msg2="发送完毕";
    sendn(task->peerfd,msg2,strlen(msg2));
    close(fd);  
}

