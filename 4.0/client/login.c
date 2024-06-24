#include "client.h"
/* #include "str_util.h" */
#include <iso646.h>
#include <stdalign.h>

int login_client(int sockfd, User* user, char* Token){
    //1.发送用户名
    char username[64] = {0};
    printf("请输入用户名: ");
    scanf("%s",username);

    //登录第一次交互，只填充User结构体的用户名
    strncpy(user->userName, username, sizeof(user->userName)); //2填充用户名
    strcpy(user->salt, "salt");
    strcpy(user->cryptpasswd, "cryptpasswd");
    strcpy(user->pwd, "pwd");
    
    //填充并发送小火车协议
    train_t t;
    memset(&t, 0, sizeof(t));
    t.len = 0;
    t.type = CMD_TYPE_LOGIN_USRNAME;
    t.user = *user;
    send(sockfd, &t, 4 + 4 + sizeof(User) + t.len, 0);
    /* printf("发小火车1:发送用户名\n"); */

    // 接收服务器发来回应:有用户名，返回盐值。无用户返回失败
    //接小火车
    int len = 0;
    recvn(sockfd, &len, sizeof(len));
    CmdType cmdType;
    recvn(sockfd, &cmdType, sizeof(cmdType));
    int ret = recvn(sockfd, user, sizeof(User));
    /* printf("接小火车1:ret = %d, cmdType = %d, user信息:id = %d,userName = %s, salt = %s\n, cryptpasswd = %s, pwd = %s", */
               /* ret,cmdType, user->id, user->userName, user->salt, user->cryptpasswd, user->pwd); */
    
    //2.若用户名存在,接收盐值,输入密码，发送密码
    if(cmdType == MSG_TYPE_LOGIN_SALT){
        /* printf("用户存在\n"); */       
        /* printf("收到的user->salt:%s\n", user->salt); */
        char password[64] = {0};
        printf("请输入密码: ");
        scanf("%s",password);
        //用盐值加密密码
        char encrypted_password[65] = {0};
        encrypt_password(password, user->salt, encrypted_password);
        
        //填充小火车协议
        train_t t;
        memset(&t, 0, sizeof(t));
        t.len = 0;
        t.type = CMD_TYPE_LOGIN_ENCRYTPTEDCODE;
        t.user = *user;
        //把加密后的密码赋给小火车的user加密
        strcpy(t.user.cryptpasswd, encrypted_password); 
        send(sockfd, &t, 4 + 4 + sizeof(User) + t.len, 0);
        /* printf("发小火车2：发送加密后的密码:%s\n",t.user.cryptpasswd); */
    
        //等待服务器回应:对比加密密码与数据库中的是否一致
        //接小火车
        int len = 0;
        recvn(sockfd, &len, sizeof(len));
        CmdType cmdType;
        recvn(sockfd, &cmdType, sizeof(cmdType));
        ret = recvn(sockfd, user, sizeof(User));
        /* printf("接小火车2:ret = %d,cmdType = %d, user信息:id = %d,userName = %s, salt = %s\n, cryptpasswd = %s, pwd = %s\n", */
               /* ret, cmdType, user->id, user->userName, user->salt, user->cryptpasswd, user->pwd); */
        
        if(cmdType == MSG_TYPE_LOGINOK){
            printf("--------------------------\n");
            printf("密码正确。\n");

            //接收Token:先接长度，再接内容
            recvn(sockfd, &len, sizeof(len));
            recvn(sockfd, Token, len);
            
            return 1;
        }else{
            printf("--------------------------\n");
            printf("密码错误。\n");
            return 0;
        }
    }else{
        printf("--------------------------\n");
        printf("该用户名不存在。\n");
        return 0;
    }
}
