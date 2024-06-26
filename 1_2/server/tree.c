#include "thread_pool.h"

#define ARGS_CHECK_TREE(x,y){\
    if(x != y)error(-1,0,"./tree filename\n");\
}

int directories = 0, files = 0;
char treestr[4096];

/* width: 缩进的空格数目*/
void dfs_print(const char* path, int width){
    //打开目录流
    DIR* stream = opendir(path);
    if(!stream)error(-1,errno,"opendir %s",path);
    //遍历每一目录项
    errno = 0;
    struct dirent* pdirent;
    while((pdirent = readdir(stream)) != NULL){
        char* filename = pdirent->d_name;
        //忽略隐藏文件
        if(filename[0] == '.')continue;
        //打印文件名
        strcat(treestr,"|");
        for(int i = 0; i < width; i++){
            if(i < width-4)
                strcat(treestr," ");
            else if(i!= 0 && i%4==0)
                strcat(treestr,"|");
            else 
                strcat(treestr,"_");
        }
        strcat(treestr, filename);
        strcat(treestr, "\n");
        //递归继续打印
        if(pdirent->d_type == DT_DIR){
            directories++;
            //拼接路径
            char subpath[128];
            sprintf(subpath,"%s/%s",path,filename);
            dfs_print(subpath,width+4);
        }else{
            files++;
        }
    }

    if(errno)
        error(1,errno,"readdir");

    closedir(stream);
}

void treeCommand(task_t* task)
{   
    int socketfd = task->peerfd;
    char * dirname = task->data;
    if(strcmp(dirname,"") == 0)
        dirname = getcwd(NULL, 0);
    // ./tree dir
    memset(treestr, 0, sizeof(treestr));
    strcpy(treestr, dirname); // 打印目录的名字
    strcat(treestr, "\n"); //
    dfs_print(dirname, 4); // 递归打印每一个目录项
    //返回结果发送给客户端
    sendn(socketfd, treestr, strlen(treestr));
}


