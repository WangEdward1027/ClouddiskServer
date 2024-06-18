#include "thread_pool.h"

//0777 是创建目录的权限设置
#define MODE 0777

void mkdirCommand1(task_t * task){
    
    const char *Dir_Name=task->data;
    char buff[128];
    bzero(buff,sizeof(buff));

    //// 尝试创建目录
    if(mkdir(Dir_Name,MODE)==-1){
        
        // 检查创建失败是否因为目录已存在
        if(errno==EEXIST){
            sprintf(buff,"Directory '%s' already exists.\n",Dir_Name);
            sendn(task->peerfd,buff,sizeof(buff));
            return;
        }

        //其他创建失败
        else{
            sprintf(buff,"Create dir Failed.\n");
            sendn(task->peerfd,buff,sizeof(buff));

            syslog(LOG_WARNING,"User execute MKDIR command: Failed.");
            return;
        }
    } 
    else{
        //创建成功
        sprintf(buff,"Create dir Sucessful.");
        sendn(task->peerfd,buff,sizeof(buff));
        syslog(LOG_INFO,"User execute MKDIR command: Successful.");
        return;
    }
}
