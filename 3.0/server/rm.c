/*甘博*/

#include"thread_pool.h"


void removeCommand(task_t *task){
    //接收任务中的目录
    printf("开始删除");
    if(strlen(task->data)==0){
        const char *msg = "请输入文件名";
        sendn(task->peerfd, msg, strlen(msg));
        return;
    }
    char filename[1000];
    strncpy(filename,task->data,sizeof(filename)-1);
    filename[sizeof(filename)-1]='\0';//确保字符串以null结尾
    
    struct stat st;
    if(stat(filename,&st)==-1){
        if(errno==ENOENT){
            const char *msg="文件不存在";
            sendn(task->peerfd,msg,strlen(msg));
        }else{
            perror("stat");
            const char *msg="无法获取文件信息";
            sendn(task->peerfd,msg,strlen(msg));
        }
        return;
    }

    if(!S_ISREG(st.st_mode)){
        const char *msg="目标不是一个文件";
        sendn(task->peerfd,msg,strlen(msg));
        return;
    }

    //删除文件
    if(remove(filename)==0){
        const char *successMsg = "删除成功";
        sendn(task->peerfd, successMsg, strlen(successMsg));

    }else{
        perror("remove");
        const char *msg="删除文件失败";
        sendn(task->peerfd,msg,strlen(msg));
    }
}

