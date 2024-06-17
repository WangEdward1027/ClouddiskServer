/*甘博*/

#include"thread_pool.h"

void rmDirRec(const char *path)
{
    //打开目录
    DIR *dp=opendir(path);
    if(dp==NULL)
    {
        error(1,errno,"opendir");
    }
    //遍历目录流，递归删除每一项目录项
    errno=0;
    struct dirent*pdir;
    while((pdir=readdir(dp))!=NULL){
        //忽略.和..
        char *name =pdir->d_name;
        if(strcmp(name,".")==0||strcmp(name,"..")==0){
            continue;
        }

        //拼接路径
        char subpath[1024];
        sprintf(subpath,"%s/%s",path,name);
        
        //递归删除
        if(pdir->d_type==DT_DIR){
            rmDirRec(subpath);
        }else if(pdir->d_type==DT_REG){
            unlink(subpath);
            /*对于硬链接来说，unlink 用来删除目录项，并把 inode 引用计数减 1，这两步也是一个原子过程。
             * 直到 inode 引用计数为 0，才会真正删除文件。
             对于软链接来说，unlink 直接删除软链接，而不影响软链接指向的文件。*/
        }
    }//pdir==NULL
    closedir(dp);

    if(errno){
        error(1,errno,"readdir");
    }
    //再删除目录
    rmdir(path);
}

void rmdirComand(task_t *task){
    //接收任务中的目录
    char dirPath[1000];
    strncpy(dirPath,task->data,sizeof(dirPath)-1);
    dirPath[sizeof(dirPath)-1]='\0';//确保字符串以null结尾
    
    //递归删除函数
    rmDirRec(dirPath);
}
