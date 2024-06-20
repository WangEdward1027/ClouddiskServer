#include "thread_pool.h"

void pwdCommand(task_t * task)
{
    char* path;
    char buff[512];
    path = task->user->pwd;
    
    strcpy(buff, path);
    strcat(buff, "\n");
    sendn(task->peerfd, buff, strlen(buff));
    
    return;
}
