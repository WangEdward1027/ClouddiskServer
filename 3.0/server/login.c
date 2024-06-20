#include "thread_pool.h"
#include <openssl/rand.h>
#define SALT_LENGTH 16

//生成盐值的函数在注册 register.c中


void userLoginCheck1(task_t * task)
{
    char username[64];
    
    sscanf(task->data,"CMD_TYPE_REGISTER_USERNAME:%s",username);
    
    if(selectUserByUserName(username)){
        snprintf(task->data,sizeof(task->data),"USER_EXISTS");
    }else{
        char salt[SALT_LENGTH*2+1];
               
        generateSalt(salt,SALT_LENGTH);
        
        strncpy(task->user->userName,username,sizeof(task->user->userName));

        snprintf(task->data,sizeof(task->data),"SALT:%s",salt);
        //将盐值保存以便后续使用
        strncpy(task->user->salt,salt,sizeof(task->user->salt));
    }
    send(task->peerfd,task->data,strlen(task->data),0);
}

void userLoginCheck2(task_t * task)
{

}
