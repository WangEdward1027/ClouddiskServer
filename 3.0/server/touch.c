#include "thread_pool.h"
#include <openssl/md5.h>
#define MODE 0644

void touchCommand(task_t * task)
{
    //获取新建的文件名
    char * fileName=task->data;
    User user={0};
    //获取用户userid
    user.id=selectUserByUserName(task->user->userName)->id;
    //当前用户路径
    strcpy(user.pwd,task->user->pwd);

    //发送缓冲区
    char buff[128];
    bzero(buff,sizeof(buff));

    //全路径
    char fullPath[1024];
    strcpy(fullPath,user.pwd);
    strcpy(fullPath,fileName);
    //snprintf(fullPath,sizeof(fullPath),"%s%s",user.pwd,task->data);
    //FileEntry *entry=getEntryByPath(fullPath);
    

    int fd=open(fileName,O_CREAT,MODE);
    if(fd!=-1){
        //获取MD5
        FileEntry *Flenry=(FileEntry *)calloc(1,sizeof(FileEntry));
        char md5sum[MD5_DIGEST_LENGTH];
        generateMD5(fullPath,md5sum);
        strcpy(Flenry->md5,md5sum);

        //获取文件大小
        struct stat fileStats;
        if (stat(fullPath, &fileStats) == -1) {
            perror("Error getting file stats");
            return;
        }
        else {
            Flenry->fileSize=fileStats.st_size;
        }
        //文件类型
        Flenry->fileType=1;
        
        //获取parent_id
        char Temp[128];
        strcpy(Temp,getCurrentDirectory(task->user->pwd));

        int entryCount[7]={0};
        FileEntry *Tempfl=(FileEntry *)calloc(1,sizeof(FileEntry));

        Tempfl=selectFileEntryByFileNameAndOwnerId(Temp,user.id,entryCount);
        Flenry->parentId=Tempfl->parentId;
        //获取文件名
        strcpy(Flenry->fileName,user.userName);
        //用户名
        Flenry->ownerId=user.id;

        //结构体拼装完成，调用函数
        int res=addFileEntry(Flenry);
        if(res){
            sprintf(buff,"创建文件 '%s' 失败.",fileName);
            sendn(task->peerfd,buff,sizeof(buff));
            close(fd);
            return;
        }
        sprintf(buff,"创建文件 '%s' 成功.",fileName);
        sendn(task->peerfd,buff,sizeof(buff));
        close(fd);
        return;
    }
    else{
//        if(errno==EEXIST){
//            sprintf(buff,"文件 '%s' 已经存在.\n",fileName);
//            sendn(task->peerfd,buff,sizeof(buff));
//            close(fd);
//            return;
//        }

        sprintf(buff,"创建文件 '%s' 失败.\n",fileName);
        sendn(task->peerfd,buff,sizeof(buff));
        close(fd);
        return;
    }

}
