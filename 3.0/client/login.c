#include "client.h"
/* #include "str_util.h" */
#include <iso646.h>
#include <stdalign.h>
void print_progress(int progress) {
    int bar_width = 50;
    int pos = bar_width * progress / 100;
    printf("\r\033[1;91m[");
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) printf("█");
        else printf(" ");
    }
    printf("] %d%%\033[0m", progress);
    fflush(stdout);
}
int login_client(int sockfd, User* user){
    //1.发送用户名
    char username[64] = {0};
    printf("\033[1;97m主人的用户名: \033[0m");
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
        printf("\033[1;97m主人的密码: \033[0m");
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
             int i;
                        
            printf("\n\033[1;96m正在登录中请稍等。。。。。\033[0m\n");
             for (i = 0; i < 99; i++) {
                print_progress(i);
                usleep(10000); // 50ms 延迟

                // 在某些进度时卡顿一下
                if (i == 30 || i == 60 || i == 90) {
                usleep(100000); // 500ms 延迟
                }
            }

            // 99%的时候卡住
            print_progress(99);
            usleep(1000000); // 5秒延迟
            printf("\n");
            printf("\n\033[1;30;47;5m不是哥们，真有人在等进度条啊，我路过的\033[0m\n");
            printf("\n");
            usleep(100000);
            printf("\033[1;96m------------------------------------\033[0m \n");
            printf("\033[1;96m密码正确!太棒了，主人！\033[0m\n");
            return 1;
        }else{
            printf("\033[1;96m-----------------------------------\033[0m \n");
            printf("\033[1;96m密码错误,密码怎么忘记了呢？\033[0m\n");
            return 0;
        }
    }else{
            printf("\033[1;96m-----------------------------------\033[0m \n");
        printf("\033[1;96m用户名不存在。不注册还想撸我？？\033[0m\n");
        return 0;
    }
}
