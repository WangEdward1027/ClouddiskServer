#include "thread_pool.h"

#define MODE 0644

void touchCommand(task_t *task){
    
    const char * Dir_Name =task->data;
    char buff[128];
    bzero(buff,sizeof(buff));

    int fd=open(Dir_Name,O_CREAT,MODE);
    if(fd!=-1){

        sprintf(buff,"创建文件 '%s' 成功.",Dir_Name);
        sendn(task->peerfd,buff,sizeof(buff));
        close(fd);
        return;
    }
    else{
        if(errno==EEXIST){
            sprintf(buff,"文件 '%s' 已经存在.\n",Dir_Name);
            sendn(task->peerfd,buff,sizeof(buff));
            close(fd);
            return;
        }

        sprintf(buff,"创建文件 '%s' 失败.\n",Dir_Name);
        sendn(task->peerfd,buff,sizeof(buff));
        close(fd);
        return;
    }
}

