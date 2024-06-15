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
        
        递归删除
        if(pdir->d_type==DT_DIR){
            rmDirRec(subpath);
        }else if(pdir->d_type==DT_REG){
            unlink(subpath);
        }
    }//pdir==NULL
    closedir(dp);

    if(errno){
        error(1,errno,"readdir");
    }
    //再删除目录
    rdmir(path);
}

void rmdirComand(task_t *task){
    //接收任务中的目录
    char dirPath[1000];
    strncpy(dirPath,task->data,sizeof(dirPath)-1);
    dirPath[sizeof(dirPath)-1]='\0';//确保字符串以null结尾
    
    //递归删除函数
    rmDirRec(dirPath);
}
