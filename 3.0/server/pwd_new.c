#include "thread_pool.h"

void pwdCommand_new(task_t * task) {

    // 1. 获取当前username
    char * username = task->user->userName;

    // 2. 根据当前username查到对应的user_info得到对应的pwd
    User * user = selectUserByUserName(username);

    // 3. 获取当前的pwd
    char * path = user->pwd;

    char buff[64];
    strcpy(buff, path);
    
    // 4. 发送给客户端信息
    sendn(task->peerfd, buff, sizeof(buff));

}
