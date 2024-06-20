#include"thread_pool.h"

//递归删除目录及文件
void recRemoveDir(int dirId){
    //获取指定目录ID下的所有文件和子目录
    FileEntry *entries =getEntriesInDir(dirId);
    if(entries==NULL)
        return;

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
    if(strlen(task->data)==0){
        const char *msg="请输入目录";
        sendn(task->peerfd,msg,strlen(msg));
        return;
    }

    //拼接用户当前工作目录和用户输入目录，用以检查是否存在
    char fullPath[2000];
    snprintf(fullPath,sizeof(fullPath),"%s/%s",task->user->pwd,task->data);
    
    //获取完整路径对应的条目
    FileEntry*entry=getEntryByPath(fullPath);
    if(entry==NULL){
        const char*msg="目录不存在";
        sendn(task->peerfd,msg,strlen(msg));
        return;
    }

    //检查条目类型，如果是文件返回错误信息
    if(entry->fileType==1){
        const char *msg="此操作对文件无效";
        sendn(task->peerfd,msg,strlen(msg));
        return;
    }

    //递归删除目录及内容
    recRemoveDir(entry->id);

    //删除成功
    const char *msg="目录删除成功";
    sendn(task->peerfd,msg,strlen(msg));
    
}

