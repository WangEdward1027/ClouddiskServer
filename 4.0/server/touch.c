#include "thread_pool.h"
#include <openssl/md5.h>
#define MODE 0644

void touchCommand(task_t * task)
{   
    removeTrailingSpace(task->data); 

    printf("_______________________开始创建文件\n");
    //获取新建的文件名
    char * fileName=task->data;
    printf("--------------------文件名%s\n",fileName);
    User user={0};
    //获取用户userid
    user.id=selectUserByUserName(task->user->userName)->id;
    printf("--------------------用户的id：%d\n",user.id);
    //当前用户路径
    char*pwd=getUserPWD(task->user->userName);
    strcpy(user.pwd,pwd);
    printf("-------------------当前工作路径%s\n",user.pwd);
    //发送缓冲区
    char buff[128];
    bzero(buff,sizeof(buff));

    //全路径
    char fullPath[1024];
    strcpy(fullPath,user.pwd);
    strcat(fullPath,"/");
    strcat(fullPath,fileName);
    printf("----------------工作路径：%s\n",fullPath);
    //snprintf(fullPath,sizeof(fullPath),"%s%s",user.pwd,task->data);
    //FileEntry *entry=getEntryByPath(fullPath);
    

    int fd=open(fileName,O_CREAT| O_WRONLY,MODE);
    //sleep(30);
    printf("asddasdas:filename:%s\n",fileName);
    printf("----------------------fd:%d\n",fd);
    if(fd!=-1){
        //获取MD5
        FileEntry *Flenry=(FileEntry *)calloc(1,sizeof(FileEntry));
        char md5sum[MD5_DIGEST_LENGTH];
        generateMD5(fileName,md5sum);
        strcpy(Flenry->md5,md5sum);
        printf("------------------md5:%s\n",md5sum);

        //获取文件大小
        struct stat fileStats;
        if (stat(fileName, &fileStats) == -1) {
            perror("Error getting file stats");
            return;
        }
        else {
            printf("文件大小：%d\n",Flenry->fileSize);
            Flenry->fileSize=fileStats.st_size;
        }
        //文件类型
        Flenry->fileType=1;
        
        //获取parent_id
        //char Temp[128];
        //strcpy(Temp,getCurrentDirectory(task->user->pwd));

        //int entryCount[7]={0};
        //FileEntry *Tempfl=(FileEntry *)calloc(1,sizeof(FileEntry));
        printf("用户pwd：%s\n",user.pwd);
        FileEntry*Tempfl=getEntryByPath(user.pwd);
        printf("id：%d\n",Tempfl->id);
        //Tempfl=selectFileEntryByFileNameAndOwnerId(Temp,user.id,entryCount);
        Flenry->parentId=Tempfl->id;
        //获取文件名
        strcpy(Flenry->fileName,fileName);

        printf("用户名：%s\n",Flenry->fileName);
        //用户名
        Flenry->ownerId=user.id;

        //结构体拼装完成，调用函数
        int res=addFileEntry(Flenry);
        if(res){
            sprintf(buff,"创建文件 '%s' 失败.",fileName);
            sendn(task->peerfd,buff,sizeof(buff));
            close(fd);
            free(Tempfl);
            free(Flenry);
            return;
        }
        sprintf(buff,"创建文件 '%s' 成功.",fileName);
        sendn(task->peerfd,buff,sizeof(buff));
        close(fd);
        free(Tempfl);
        free(Flenry);
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
