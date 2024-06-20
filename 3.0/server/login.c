#include "thread_pool.h"
#include <openssl/rand.h>
#define SALT_LENGTH 16

//用户登录1: 接收用户名，对比用户表该用户是否存在。存在发盐值，不存在不发。
void userLoginCheck1(task_t * task)
{
    
    char username[64];
    sscanf(task->data,"CMD_TYPE_REGISTER_USERNAME:%s",username);
    
    //若用户名存在,发送该用户对应的盐值
    User* user = selectUserByUserName(username);
    if(user != NULL){
        task->user->id = user->id; //1填用户id
        strncpy(task->user->salt, user->salt, sizeof(task->user->salt)); //3填盐值
        send(task->peerfd, task->user->salt, strlen(task->user->salt), 0);
    }
    //若用户名不存在,返回用户名不存在,不发送盐值
    else{
        snprintf(task->data,sizeof(task->data),"用户名不存在");
        send(task->peerfd, task->data, strlen(task->data), 0);
    }
}

//用户登录2:验证密码
void userLoginCheck2(task_t * task)
{
    //接收密码
    char encrypted_password[64];
    sscanf(task->data,"CMD_TYPE_ENCRYPTECODE:%s",encrypted_password);

    //去数据库中获取密码,进行对比
        //准备操作:保存用户名，待会用用户名查询用户表
        char username[64];
        sscanf(task->data,"CMD_TYPE_REGISTER_USERNAME:%s",username);
        User* user = selectUserByUserName(username);
    //若密码相同，则返回登录成功
    if(encrypted_password == user->cryptpasswd){
        snprintf(task->data,sizeof(task->data),"MSG_TYPE_LOGINOK");
        strncpy(task->user->cryptpasswd, user->cryptpasswd, sizeof(task->user->cryptpasswd)); //4填加密密码
        //5填当前目录
        //需要一个接口
        //to do
    }
    //若密码不同，则返回登录失败
    else{
        snprintf(task->data,sizeof(task->data),"MSG_TYPE_LOGINERROR");
    }
    send(task->peerfd,task->data,strlen(task->data),0);
}
