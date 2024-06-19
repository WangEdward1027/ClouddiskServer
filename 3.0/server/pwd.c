
/*
 *@author zzj created
 *
 *
 *
 */

#include "thread_pool.h"

void pwdCommand(task_t * task)
{
    
    char* cwd;
    char buff[1024] = {0};
    bzero(buff, sizeof(buff));
    if ((cwd = getcwd( NULL, 0)) == NULL) {
        error( 1, errno, "getcwd failed");
    }
    
    // 将获取的pwd保存到buff中
    strcpy(buff, cwd);
    buff[strlen(buff)] = '\0';

    free(cwd);
    
    // 将buff的内容发送给客户端
    sendn(task->peerfd, buff, strlen(buff) + 1);

}

