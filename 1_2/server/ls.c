/*
 *@author lwh created
 *
 *
 *
 */
#include "thread_pool.h"

void lsCommand(task_t * task)
{
    char* cwd; // 用于保存用户传入的目录
    char buff[1024]; // 用于保存要发送给客户端的内容
    bzero(&buff, sizeof(buff));

    if (strcmp(task->data, "") == 0) {
        // 用户未传入任何内容

        cwd = getcwd(NULL, 0); // 获取当前工作目录的绝对路径

        if (cwd == NULL) {
            // 获取当前目录失败
            sprintf(buff, "getcwd failed");
            sendn(task->peerfd, buff, sizeof(buff));
            return;
        }
    }else {
        cwd = task->data;
    }
    

    DIR* stream = opendir(cwd); // 打开当前所在目录的目录流
    if (!stream) {
        // 打开目录失败
        sprintf(buff, "opendir %s failed, %s not exist.\n", cwd, cwd);
        sendn(task->peerfd, buff, sizeof(buff));
        return;
    }

    struct dirent* pdirent; // 结构体用于保存当前目录信息
    
    int i = 0;

    while((pdirent = readdir(stream)) != NULL) {
        char* filename = pdirent->d_name;

        // 忽略.和..
        if ((strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)) {
            continue;
        }
        
        // 忽略隐藏文件
        if (filename[0] == '.') {
            continue;
        }

        
        // 打印目录项名字
        // puts(filename); 这里不用puts的原因是 puts属于行缓冲，
        // 每次输出都会在末尾自动加一个换行符，不符合ls命令的形式 
        // printf("%-15s\t", filename);
        sprintf(buff, "%-15s\t", filename);

        i++;
        if (i % 4 == 0) {
            // printf("\n");   // 为了美观设置每输出4个单词就换行
            sprintf(buff, "\n");
        }
        
    }
    
    // printf("\n");
    sprintf(buff, "\n");
    
    closedir(stream);
    
    // 将服务器端ls产生的内容，发送到客户端
    sendn(task->peerfd, buff, sizeof(buff));

}

