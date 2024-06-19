#include "thread_pool.h"

//0777 是创建目录的权限设置
#define MODE 0777

void mkdirCommand(task_t * task){
    
    const char *Dir_Name=task->data;
    char buff[128];
    bzero(buff,sizeof(buff));

    //尝试创建目录
    if(mkdir(Dir_Name,MODE)==-1){
        // 检查创建失败是否因为目录已存在
        if(errno==EEXIST){
            sprintf(buff,"目录 '%s' 已经存在.\n",Dir_Name);
            sendn(task->peerfd,buff,sizeof(buff));
            return;
        }

        //其他创建失败
        else{
            sprintf(buff,"创建目录  '%s'  失败.\n",Dir_Name);
            sendn(task->peerfd,buff,sizeof(buff));
            syslog(LOG_WARNING,"User execute MKDIR command: Failed."); //日志
            return;
        }
    } 
    else{
        //目录创建成功
        sprintf(buff,"创建目录 '%s' 成功.",Dir_Name);
        sendn(task->peerfd,buff,sizeof(buff));
        syslog(LOG_INFO,"User execute MKDIR command: Successful."); //日志
        return;
    }
}
