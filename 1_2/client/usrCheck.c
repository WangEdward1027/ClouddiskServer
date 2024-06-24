#include "client.h"

/*//客户端发送信息
int sendMessage(int sockfd, char* buffer, CmdType cmdType){

    int ret;
    int len = strlen(buffer);
    //1.1先发消息长度
    ret = sendn(sockfd, (char*)&len, sizeof(len));
    ERROR_CHECK(ret, -1, "sendMessage len");
    //1.2再发消息类型
    ret = sendn(sockfd, &cmdType, sizeof(CmdType));
    ERROR_CHECK(ret, -1, "sendMessage CmdType");
    //1.3最后发消息内容
    ret = sendn(sockfd, buffer, len);
    ERROR_CHECK(ret , -1, "sendMessage data");

    return 0;
}

//客户端接收信息
int recvMessage(int sockfd, char* buffer){
    //1.1 获取消息长度
    int length = -1;
    int ret = recvn(sockfd, &length, sizeof(length));
    ERROR_CHECK(ret, -1, "recvMessage");
    printf("recv length: %d\n", length);

    //1.2 获取消息类型
    CmdType cmdType;
    ret = recvn(sockfd, &cmdType, sizeof(cmdType));
    printf("recv cmd type: %d\n", cmdType);
    //1.3 获取消息内容
    ret = recvn(sockfd, buffer, length);

    return 0;
}*/

static int userLogin1(int sockfd, train_t *pt)
{
    printf("userLogin1.\n");
    train_t t;
    memset(&t, 0, sizeof(t));
    while(1) {
        printf(USER_NAME);
        char user[20]= {0};
        int ret = read(STDIN_FILENO, user, sizeof(user));
        user[ret - 1] = '\0';
        t.len = strlen(user);
        t.type = TASK_LOGIN_SECTION1;
        strncpy(t.buff, user, t.len);
        ret = sendn(sockfd, &t, 8 + t.len);
        /* printf("login1 send %d bytes.\n", ret); */

        //接收信息
        memset(&t, 0, sizeof(t));
        ret = recvn(sockfd, &t.len, 4);
        /* printf("length: %d\n", t.len); */
        ret = recvn(sockfd, &t.type, 4);
        if(t.type == TASK_LOGIN_SECTION1_RESP_ERROR) {
            //无效用户名, 重新输入
            printf("user name not exist.\n");
            continue;
        }
        //用户名正确，读取setting
        ret = recvn(sockfd, t.buff, t.len);
        break;
    }
    memcpy(pt, &t, sizeof(t));
    return 0;
}

static int userLogin2(int sockfd, train_t * pt)
{
    printf("userLogin2.\n");
    int ret;
    train_t t;
    memset(&t, 0, sizeof(t));
    while(1) {
        char * passwd = getpass(PASSWORD);
        /* printf("password: %s\n", passwd); */
        char * encrytped = crypt(passwd, pt->buff);
        /* printf("encrytped: %s\n", encrytped); */
        t.len = strlen(encrytped);
        t.type = TASK_LOGIN_SECTION2;
        strncpy(t.buff, encrytped, t.len);
        ret = sendn(sockfd, &t, 8 + t.len);
        /* printf("userLogin2 send %d bytes.\n", ret); */ 

        memset(&t, 0, sizeof(t));
        ret = recvn(sockfd, &t.len, 4);
        /* printf("2 length: %d\n", t.len); */
        ret = recvn(sockfd, &t.type, 4);
        if(t.type == TASK_LOGIN_SECTION2_RESP_ERROR) {
            //密码不正确
            printf("sorry, password is not correct.\n");
            continue;
        } else {
            ret = recvn(sockfd, t.buff, t.len);
            printf("Login Success.\n");
            printf("please input a command.\n");
            fprintf(stderr, "%s", t.buff);
            break;
        } 
    }
    return 0;
}

//用户校验
int usrCheck(int sockfd)
{
   /* //客户端输入用户名发送给服务器
    char usrname[128];
    printf("请输入登录用户名：");
    scanf("%s",usrname);
    sendMessage(sockfd, usrname, CMD_TYPE_USRNAME);
    
    //接收服务器端返回的盐值
    char setting[128] = {0};
    recvMessage(sockfd, setting);
    
    //客户输入密码
    char password[128] = {0};
    printf("\n请输入密码：");
    scanf("%s",password);
    
    //将密码与盐值一起加密发送给服务器
    char* encoded = crypt(password, setting);
    sendMessage(sockfd, encoded, CMD_TYPE_USRENCODE);
    
    //接收服务器的校验结果
    int result;
    recvMessage(sockfd, (char*)&result);

    return result;*/
    //techer version
    train_t t;
    memset(&t, 0, sizeof(t));
    userLogin1(sockfd, &t);
    userLogin2(sockfd, &t);
    return 0;
}


