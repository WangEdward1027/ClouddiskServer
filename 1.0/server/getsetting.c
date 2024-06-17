#include "thread_pool.h"
#include <shadow.h>

void get_setting(char *salt,char *passwd)
{
    int i,j;
    //取出salt,i 记录密码字符下标，j记录$出现次数
    for(i = 0,j = 0;passwd[i]&& j!=4;++i)
    {
        if(passwd[i] == '$')
            ++j;
    }
    strncpy(salt,passwd,i);
}

void getsetting(task_t * task)
{
    //获取任务
    int socketfd = task->peerfd;
    //获取客户端发来的用户名
    char usrname[128];
    strcpy(usrname, task->data);
    //查找对应用户的盐值
    struct spwd *sp;
    char setting[512] = {0};
    if((sp = getspnam(usrname))==NULL){
        //错误处理
        printf("Don't have usrname:%s",usrname);
        return ;
    }
   //获取盐值
    get_setting(setting,sp->sp_pwdp);
    //将盐值发送给客户端
    sendMessage(socketfd, setting, CMD_TYPE_USRNAME);
}

void sendMessage(int sockfd, char* buffer, CmdType cmdType){    
    int ret;
    int len = strlen(buffer);
    //1.1先发消息长度
    sendn(sockfd, (char*)&len, sizeof(len));
    //1.2再发消息类型
    sendn(sockfd, &cmdType, sizeof(CmdType));
    //1.3最后发消息内容
    sendn(sockfd, buffer, len);
}

