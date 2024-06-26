#include "thread_pool.h"

void cdCommand(task_t* task) {
    // 保存用户要切换的目录
    char* path = task->data;
    char buff[4096];
    bzero(buff, sizeof(buff));
    
    // 切换到相应的目录
    if (chdir(path) == -1) {
        // 切换失败
        sprintf(buff, "切换目录失败, %s不存在\n", path);
        sendn(task->peerfd, buff, sizeof(buff));
        return;
    }

    // 切换成功，打印新的当前工作目录
    char cwd[2048];
    bzero(cwd, sizeof(cwd));

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        // 获取当前工作目录成功
        sprintf(buff, "切换目录成功!\n当前工作目录:%s\n", cwd);
    } 
    // 发送buff到客户端
    sendn(task->peerfd, buff, sizeof(buff));
}
