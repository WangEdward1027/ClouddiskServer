#include "thread_pool.h"
#include <openssl/rand.h>
#define SALT_LENGTH 16

void sendUser(int socket, User *user) {
    printf("sendUser usename: %s\n", user->userName);
    if (sendn(socket, user, sizeof(User)) == -1) {
        perror("send");
        exit(EXIT_FAILURE);
    }
}

//用户登录1: 接收用户名，对比用户表该用户是否存在。存在发盐值，不存在不发。
void userLoginCheck1(task_t * task)
{
    printf("--------------- userLoginCheck1\n");
    char username[64];
    sscanf(task->user->userName,"%s",username);//handleMessage中用小火车接收
    printf("userLoginCheck1()收到username = %s\n",username);

    //若用户名存在,发送该用户对应的盐值
    User* user = selectUserByUserName(username);
    if(user != NULL){
        printf("用户名存在,用户id = %d,盐值 = %s\n",user->id, user->salt);
        task->user = user;  //赋给服务器的user
        //小火车
        train_t t;
        memset(&t, 0, sizeof(t));
        t.len = 0;
        t.type = MSG_TYPE_LOGIN_SALT;  //盐值
        t.user = *user;
        strcpy(t.user.cryptpasswd,"不告诉你密码");
        printf("用小火车发盐值1: cmdType = %d, user信息:id = %d,userName = %s, salt = %s\n, cryptpasswd = %s, pwd = %s\n",
           t.type, t.user.id, t.user.userName, t.user.salt, t.user.cryptpasswd, t.user.pwd);
        send(task->peerfd, &t, 4 + 4 + sizeof(User) + t.len, 0);
    }
    //若用户名不存在,返回用户名不存在,不发送盐值
    else{
        printf("用户名不存在\n");
        //TO DO 思考，用户不存在，两边怎么安全结束
        //小火车
        train_t t;
        memset(&t, 0, sizeof(t));
        t.len = 0;
        t.type = MSG_TYPE_LOGINERROR;  //登录错误
        //t.user = *user;  //user == NULL, *user可能程序崩溃

        send(task->peerfd, &t, 4 + 4 + sizeof(User) + t.len, 0);
    }
    printf("---------------- userLoginCheck1执行完毕\n");
}

//用户登录2:验证密码
void userLoginCheck2(task_t * task)
{
    printf("---------------- userLoginCheck2\n");
    //handleMessage中用小火车接收
    //接收用户发来的登录密码
    //去数据库中获取密码,进行对比
    printf("接收的密码 task->user->cryptpasswd:%s\n",task->user->cryptpasswd);
    User* user =  selectUserByUserName(task->user->userName);
    printf("查表的密码 user->cryptpasswd:%s\n",user->cryptpasswd);
    
    if(strcmp(user->cryptpasswd, task->user->cryptpasswd) == 0){
        printf("加密密码对比正确\n");
        task->user = user;
        //发送小火车，通知用户登录成功
        train_t t;
        memset(&t, 0, sizeof(t));
        t.len = 0;
        t.type = MSG_TYPE_LOGINOK;  //登录成功
        t.user = *user;
        send(task->peerfd, &t, 4 + 4 + sizeof(User) + t.len, 0);
        printf("发小火车2: 通知用户登录成功\n");
    }else{
        printf("加密密码对比失败\n");
        //发送小火车，通知用户登录失败
        train_t t;
        memset(&t, 0, sizeof(t));
        t.len = 0;
        t.type = MSG_TYPE_LOGINERROR;  //登录失败
        t.user = *user;
        send(task->peerfd, &t, 4 + 4 + sizeof(User) + t.len, 0);
        printf("发小火车2: 通知用户登录失败\n");
    }
    printf("---------------- userLoginCheck2执行完毕\n");
}
