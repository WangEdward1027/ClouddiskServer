#include"thread_pool.h"

//递归删除目录及文件
void recRemoveDir(int dirId){
    //获取指定目录ID下的所有文件和子目录
    FileEntry *entries =getEntriesInDir(dirId);
    if(entries==NULL){
        printf("目录为空，删除目录：%d\n", dirId);
        deleteFileEntry(dirId);
        return;
    }
    //遍历目录下所有条目
    for(int i=0;entries[i].id!=0;i++){
        if(entries[i].fileType==1){
            //如果是文件直接删除
            deleteFileEntry(entries[i].id);
        }else{
            //如果是子目录则递归删除
            recRemoveDir(entries[i].id);
        }
    }

    //删除当前目录
    deleteFileEntry(dirId);
    free(entries);
}

void rmdirCommand(task_t *task){
    //接收任务中的目录
    removeTrailingSpace(task->data);
    if(strlen(task->data)==0){
        const char *msg="你要删目录，你倒是告诉我目录啊！！哥！";
        sendn(task->peerfd,msg,strlen(msg));
        return;
    }

    char*pwd=getUserPWD(task->user->userName);
    //拼接用户当前工作目录和用户输入目录，用以检查是否存在
    char fullPath[2000];
    snprintf(fullPath,sizeof(fullPath),"%s/%s",pwd,task->data);
    
    //获取完整路径对应的条目
    FileEntry*entry=getEntryByPath(fullPath);
    if(entry==NULL){
        const char*msg="目录都不存在，我删鸡毛";
        sendn(task->peerfd,msg,strlen(msg));
        return;
    }

    //检查条目类型，如果是文件返回错误信息
    if(entry->fileType==1){
        const char *msg="天才！删除目录的指令你拿来删文件";
        sendn(task->peerfd,msg,strlen(msg));
        return;
    }
printf("id=%d\n",entry->id);
    //递归删除目录及内容
    recRemoveDir(entry->id);

    //删除成功
    const char *msg="目录删掉了，满意了吧";
    sendn(task->peerfd,msg,strlen(msg));
    
}

