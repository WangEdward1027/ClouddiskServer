#include "thread_pool.h"
#include <openssl/rand.h>
#define SALT_LENGTH 16

//用户登录1: 接收用户名，对比用户表该用户是否存在。存在发盐值，不存在不发。
void userLoginCheck1(task_t * task)
{
    //检查task中有什么？
    /* printf("task->User->username:%s\n",task->user->userName); */

    char username[64];
    /* sscanf(task->data,"%s",username); */
    sscanf(task->user->userName,"%s",username);
    printf("测试userLoginCheck1: username:%s\n",username);

    //若用户名存在,发送该用户对应的盐值
    User* user = selectUserByUserName(username);
    if(user != NULL){
        printf("用户名存在,");
        printf("用户id:%d,",user->id);
        printf("用户userName:%s,",user->userName);
        printf("查表得到的用户盐值 salt = %s\n", user->salt);

        task->user->id = user->id; //1填用户id
        strncpy(task->user->salt, user->salt, sizeof(task->user->salt)); //3填盐值
        
        snprintf(task->data,sizeof(task->data),"SALT:%s",user->salt);
        
        printf("salt = %s\n",task->data);

        send(task->peerfd, task->data, strlen(task->data), 0);
    }
    //若用户名不存在,返回用户名不存在,不发送盐值
    else{
        printf("用户名不存在\n");
        snprintf(task->data,sizeof(task->data),"用户名不存在");
        send(task->peerfd, task->data, strlen(task->data), 0);
    }
}

//用户登录2:验证密码
void userLoginCheck2(task_t * task)
{
    printf("--------------- userLoginCheck2\n");
    //接收密码
    char encrypted_password[65];
    /* sscanf(task->user->cryptpasswd,"CMD_TYPE_ENCRYPTECODE:%s",encrypted_password); */

    strcpy(encrypted_password , task->user->cryptpasswd);
    

    //去数据库中获取密码,进行对比
        //准备操作:保存用户名，待会用用户名查询用户表
        char username[65];
        /* sscanf(task->data,"CMD_TYPE_REGISTER_USERNAME:%s",username); */ //bug在这里
        sscanf(task->user->userName,"%s",username);
        task->user = selectUserByUserName(username);
        if(task->user == NULL){
            printf("userLoginCheck2用户不存在,exit(1)\n");
            exit(1);
        }
        
        printf("用户userName:%s,",task->user->userName);
        printf("查表得到pwd: %s\n", task->user->pwd);

    //把新user的salt和pwd传递回客户端
    send(task->peerfd, task->user->pwd, strlen(task->user->pwd), 0); 
    

    //若密码相同，则返回登录成功
    /* printf("user->cryptpasswd:%s\n", user->cryptpasswd); */
    /* printf("encrypted_password:%s\n",encrypted_password); */
    if(strcmp(encrypted_password,task->user->cryptpasswd) == 0){
        printf("查表，密码相同\n");
        snprintf(task->data,sizeof(task->data),"MSG_TYPE_LOGINOK");
    }
    //若密码不同，则返回登录失败
    else{
        printf("查表，密码不同\n");
        snprintf(task->data,sizeof(task->data),"MSG_TYPE_LOGINERROR");
    }
    send(task->peerfd,task->data,strlen(task->data),0);



    printf("----------------执行完毕\n");
}
