#include "thread_pool.h"

void checkUsrEncode(task_t *task)
{
    //获取任务
    int socketfd = task->peerfd;
    //获取客户端发来的加密验证encode
    char* str = task->data;
    //分词获取用户名和encode
    char* encode = strtok(str, " ");
    char* usrname = strtok(NULL, " ");
    //获取用户的encode
    struct spwd *sp;
    sp = getspnam(usrname);
    int ret = strcmp(sp->sp_pwdp, encode);
    //发送给客户端校验结果
    sendMessage(socketfd, (char*)&ret, CMD_TYPE_USRCHECK);
}

