/*
 *@author LSQ created
 *
 *
 *
 */

#include "thread_pool.h"

//0777 是创建目录的权限设置
#define MODE 0777

void mkdirCommand(task_t * task){
    //定义了两个字符串，分别用于存储创建目录成功和失败的消息
    char message1[128] = "Create dir Sucessful!";
    char message2[128] = "Create dir Failed!";
    printf("Execute mkdir command.");
    
    //train_t 类型的变量 t 并使用 memset 函数将其所有字节初始化为0
    train_t t;
    memset(&t,0,sizeof(t));

    //使用 mkdir 函数尝试根据 task->data 指定的路径创建一个目录
    if(mkdir( task->data , MODE ) ==-1){
        //目录创建失败
        //将 message2 字符串复制到 t.buff，设置 t.len 为消息的长度
        //然后通过 sendn 函数发送消息和其长度到 task->peerfd 指定的套接字
        strcpy(t.buff,message2);
        t.len=strlen(t.buff);
        sendn(task->peerfd,&t,sizeof(t.len));

        //使用 syslog 函数记录一条警告级别的日志
        syslog(LOG_WARNING,"User execute MKDIR command: Failed.");
    } 
    else{
        //目录创建成功
        //将 message 字符串复制到 t.buff，设置 t.len 为消息的长度
        //然后通过 sendn 函数发送消息和其长度到 task->peerfd 指定的套接字
        strcpy(t.buff,message1);
        t.len=strlen(t.buff);
        sendn(task->peerfd,&t,sizeof(t.len));

        //使用 syslog 函数记录一条信息级别的日志
        syslog(LOG_INFO,"User execute MKDIR command: Successful.");
    }
}
