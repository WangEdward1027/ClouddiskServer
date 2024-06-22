#include "thread_pool.h"


void removeCommand(task_t * task){
    //接收任务中的文件名
    removeTrailingSpace(task->data);
    if(strlen(task->data)==0){
        const char *msg="输入文件名啊！懒狗！留着我帮你填吗？？！";
        sendn(task->peerfd,msg,strlen(msg));
        return;
    }
    
    char*pwd=getUserPWD(task->user->userName);
    printf("----------%s\n",pwd);
//拼接用户当前工作目录和用户输入文件，用以检查是否存在
    char fullPath[2000];
    snprintf(fullPath,sizeof(fullPath),"%s/%s",pwd,task->data);
    printf("--------fullPath:%s\n",fullPath); 
    //获取完整路径对应的条目
    FileEntry*entry=getEntryByPath(fullPath);
    if(entry==NULL){
        const char*msg="大哥文件不存在啊，你让我怎么删";
        sendn(task->peerfd,msg,strlen(msg));
        return;
    }

    //检查条目类型，如果是文件返回错误信息
    if(entry->fileType==0){
        const char *msg="oi，你那我删目录干嘛，我删文件的";
        sendn(task->peerfd,msg,strlen(msg));
        return;
    }
    //文件直接删除
    deleteFileEntry(entry->id);
    
    //删除成功
    const char *msg="文件删完了，既然一开始不需要，又何必创建它，渣男";
    sendn(task->peerfd,msg,strlen(msg));


}


    


